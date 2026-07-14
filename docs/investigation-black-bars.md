# 黒帯（アスペクト比）問題 調査メモ

## 症状

- 現在のビルドでは映像の周囲に黒帯（Debugビルドでは緑帯）が表示される
- オリジナルの頒布版ビルドでは黒帯が一切なかった
- 100%表示時にウィンドウタイトルのサイズと実際の映像サイズがずれる
- アスペクト比設定を変更しても黒帯が消えない

## 緑の枠について（別問題・解決済み）

`PCRPlayer/FrameDlg.cpp:132-135`

```cpp
#ifdef _DEBUG
    dc.FillRgn(&r, &CBrush(RGB(0, 255, 0)));  // Debug → 緑
#else
    dc.FillRgn(&r, &CBrush(RGB(0, 0, 0)));   // Release → 黒
#endif
```

**原因**: Debugビルドのデバッグ用着色。Releaseビルドにすれば黒になる。

---

## 黒帯問題の根本原因

### 原因の連鎖

```
1. FrameDlg::SizeHelper()
     gl_.getAspect(aspect)           ← WINDOW_ASPECT_DEFAULT → ネイティブサイズ取得
     getAspectRect(aspect, rc, dst)  ← アスペクト比に合わせた小矩形を計算
     setVideoPosition(dst)           ← 小矩形をEVRに渡す

2. EVRCustomPresenter::SetVideoPosition()
     SetDestinationRect(*prcDest)    ← m_rcDestRect = 小矩形

3. D3DPresentEngine::PresentSample()
     pSwapChain->Present(&m_rcDestRect, &m_rcDestRect, hwnd, ...)
                         ↑ 小矩形しかウィンドウに書き込まない

4. 結果: 小矩形の外側はD3Dに更新されない → 黒のまま
```

### 関連ファイル

| ファイル | 役割 |
|---|---|
| `PCRPlayer/FrameDlg.cpp` | `SizeHelper()` でアスペクト制約矩形を計算 |
| `PCRPlayer/Global.cpp:42-64` | `getAspect()` で `WINDOW_ASPECT_DEFAULT` → `getNativeVideoSize()` |
| `PCRPlayer/Util.cpp:653-681` | `getAspectRect()` でアスペクト制約矩形を計算 |
| `PCRPlayer/Serialize.h:152-157` | デフォルト値 `aspect(WINDOW_ASPECT_DEFAULT)` |
| `PCRPlayer/EVRCustom/PresentEngine.cpp:786` | `Present(&m_rcDestRect, &m_rcDestRect, ...)` |
| `PCRPlayer/EVRCustom/PresentEngine.cpp:999` | `copySurface` が `m_rcDestRect` に描画 |

---

## オリジナルビルドとの差異（なぜ変わったか）

### EVRCustomPresenter の2バージョン

リポジトリに旧版と新版が共存している：

| ファイル | 状態 | `SetAspectRect` |
|---|---|---|
| `Presenter_.cpp` / `_PresentEngine.cpp` | 旧版（コンパイル対象外） | **なし** |
| `Presenter.cpp` / `PresentEngine.cpp` | 新版（コンパイル対象） | **あり** |

### 変更点の詳細

**旧版 `Presenter_.cpp:897-903`**:
```cpp
CHECK_HR(hr = m_pD3DPresentEngine->SetDestinationRect(*prcDest));
{
    RECT src = { 0, 0, m_NativeVideoSize.cx, m_NativeVideoSize.cy };
    m_pD3DPresentEngine->SetSourceRect(src);  // ソース矩形を設定するだけ
}
```

**新版 `Presenter.cpp:901-905`**:
```cpp
CHECK_HR(hr = m_pD3DPresentEngine->SetDestinationRect(*prcDest));
{
    AutoLock lock(m_NativeVideoSizeLock);
    m_pD3DPresentEngine->SetAspectRect(m_NativeVideoSize);  // ← 追加された処理
}
```

`SetAspectRect()` はEVR内部でアスペクト比制約矩形 `m_rcAspectRect` を計算する機能。  
新版ではこれが追加されたが、`MFVideoARMode_None` 時のバイパスが未実装だった。

ただし実際の黒帯の原因は `m_rcAspectRect` ではなく `m_rcDestRect`（`SizeHelper` が渡す矩形）であるため、この差異は直接の原因ではない（後述）。

### 実際の原因

オリジナルビルドで黒帯がなかった理由は、デフォルトのアスペクト設定が実質的に **`WINDOW_ASPECT_NONE`**（ウィンドウに合わせる）として動作していたため。

`Serialize.h:153` のデフォルト値：
```cpp
: aspect(WINDOW_ASPECT_DEFAULT)   // 現在のコード
```

`WINDOW_ASPECT_DEFAULT` = ネイティブ映像サイズをアスペクト比として使用 → `SizeHelper` が小矩形を渡す → 黒帯。

---

## 試みた修正と結果

### 修正1: `Serialize.h` デフォルト値変更（ユーザーが元に戻した）

```cpp
// 変更前
: aspect(WINDOW_ASPECT_DEFAULT)
// 変更後
: aspect(WINDOW_ASPECT_NONE)
```

**効果**: 正しい修正。`getAspect()` が `{0,0}` を返すため `getAspectRect()` が `false` を返し、`SizeHelper` はフル矩形を渡す → 黒帯なし。  
**ただし**: 既存の設定ファイルがある場合はファイルの値が優先される。

### 修正2: `Presenter.cpp` の `SetAspectRect` 修正（効果なし）

```cpp
// 変更後
SIZE szAspect = (m_dwAspectRatioMode == MFVideoARMode_None) ? SIZE{0, 0} : m_NativeVideoSize;
m_pD3DPresentEngine->SetAspectRect(szAspect);
```

**効果なし**: `MFVideoARMode_None` の描画パスは `copySurface` → `m_rcDestRect` を使用。  
`m_rcAspectRect` を修正しても影響なし（`stretchSurface` パスのみ有効）。

---

## 描画パスの整理

```
updateSample()
  ├─ MFVideoARMode_None → copySample() → copySurface()
  │                          ResizeRect(src, m_rcSrcRect, dst, m_rcDestRect)
  │                                                             ↑ SizeHelperが渡した矩形
  │
  └─ その他              → stretchSample() → stretchSurface()
                             ResizeRect(src, m_rcSrcRect, dst, m_rcAspectRect)
                                                                ↑ SetAspectRectで計算した矩形
```

EVRCustom は `MFVideoARMode_None` に設定されているので常に `copySurface` パス。

---

## 今後の修正方針（未実施）

### 方針A: デフォルト値を `WINDOW_ASPECT_NONE` に変更（推奨・シンプル）

`PCRPlayer/Serialize.h:153` を変更：
```cpp
: aspect(WINDOW_ASPECT_NONE)
```

- `getAspect()` → `{0,0}` → `getAspectRect()` → `false` → フル矩形
- `copySurface` → `m_rcDestRect` = フル矩形 → 全面に描画
- 黒帯なし、ウィンドウに合わせて映像が引き伸ばされる
- 設定ファイルがある場合は設定ファイルの値が優先される点に注意

### 方針B: `FrameDlg::SizeHelper` のアスペクト制約を外す

`PCRPlayer/FrameDlg.cpp:30-37` を変更：
```cpp
void CFrameDlg::SizeHelper()
{
    CRect rc;
    GetClientRect(&rc);
    // アスペクト比制約をかけずフル矩形をそのまま渡す
    gl_.player.setVideoPosition(rc);
}
```

- EVR (`MFVideoARMode_None`, `copySurface`) はフル矩形に引き伸ばして描画
- 黒帯なし
- `OnPaint` での黒帯描画も不要になる（`getAspectRect` が効かなくなる）

### 方針C: EVR内部でアスペクト比を処理（大規模変更）

`PresentEngine.cpp` の `Present` をフル矩形で呼ぶ + `stretchSurface` がアスペクト比補正をかける構成にする。現在のコードでも `stretchSurface` はこの設計に近いが、`Present` の引数が `m_rcDestRect`（小矩形）になっているため、フル矩形に変更が必要。

---

## アスペクト比設定の仕組み（参考）

| 設定値 | `getAspect()` の戻り値 | 効果 |
|---|---|---|
| `WINDOW_ASPECT_DEFAULT` (0) | ネイティブ映像サイズ | ウィンドウとサイズが違うと黒帯 |
| `WINDOW_ASPECT_NONE` (1) | `{0, 0}` | ウィンドウ全体に引き伸ばし、黒帯なし |
| `WINDOW_ASPECT_CUSTOM` (2+) | ユーザー指定サイズ | 指定アスペクト比で表示 |

メニュー: 「アスペクト比 → デフォルト」= `WINDOW_ASPECT_DEFAULT`、「ウィンドウに合わせる」= `WINDOW_ASPECT_NONE`

アスペクト比変更時は `OnWindowAspectNone()` → `ApplyWindowRatio()` → `SizeHelper()` が呼ばれる。
