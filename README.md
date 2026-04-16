# PCRPlayer (modified fork)

[PCRPlayer](http://pecatv.s25.xrea.com/) の非公式フォークです。
DirectShow ベースの PeerCast 対応メディアプレイヤーで、
WMV・FLV ストリームの自動再構築や 2ch/したらば BBS の閲覧・投稿に対応しています。

このフォークでの変更点：

- 絵文字（例: ❤, 🌊）が `??` として投稿されるのを修正  
  Shift-JIS でエンコードできない文字を数値文字参照（例: `&#127754;`）に変換するようにした。（Chrome の動作に合わせた修正）

## Usage

### PeerCast 再生用コマンドライン引数

```
PCRPlayer.exe "プレイリストURL" "チャンネル名" "コンタクトURL"
```

コンタクトURL を省略しても PeerCast からコンタクトURL を取得して掲示板を読み込みますが、
チャンネル接続をしないとコンタクトURL は取得できません。
あらかじめ設定しておくと起動直後から読み込まれます。

### PeCaRecorder のプレイヤー設定

| 項目 | 値 |
|------|----|
| 引数 | `"$x" "$0" "$3"` |
| タイプ | `WMV\|FLV` |

## Features

### ステータスバー

- 項目の上で右クリックすると表示書式を選ぶメニューが表示されます
- 音量にカーソルを置くと音量スライダーが表示されます。右クリックでスライダーの表示/非表示を切り替えられます
- チャンネル名にカーソルを置くとチャンネル情報がポップアップ表示されます。情報に URL が含まれる場合は抽出されてメニューに追加されます
- チャンネル名を左ダブルクリックするとチャンネル情報を更新します（チャンネル接続中のみ）

### BBS バー

ぜろちゃんねるプラスなどの 5ch 互換掲示板形式と、したらば掲示板形式に対応しています。

- `Tab` キーで書き込み欄を表示（ショートカット初期設定の場合）
- 書き込み欄で `Shift + Enter` で書き込み（ショートカット初期設定の場合）
- 左クリックでスレッド選択ダイアログを表示
- 右クリックでメニューを表示。レスポップアップに URL が含まれる場合は抽出されてメニューに追加されます
- 右クリックメニューからスレッド自動更新カウンターを表示できます

### 内蔵 BBS ブラウザ

- 表示するまでブラウザウィンドウは生成されず、メモリを消費しません
- 表示後は非表示にしても、BBS 機能が無効になるまでブラウザウィンドウは破棄されません
- BBS 機能はローカルファイルを読み込んだときに無効になります

## Setup

### 外部デコーダ

LAV Filters や ffdshow などの外部デコーダをインストールした後、
メニューの「オプション → 詳細設定 → デコーダ」で優先して接続するデコーダを設定できます。

**LAV Filters の設定例:**

1. 「LAV Video Decoder」をビデオデコーダに、「LAV Audio Decoder」をオーディオデコーダに追加してチェックを入れます
2. 再生後、メニューの「フィルタグラフ」で設定したフィルタが使われているか確認します
3. 接続できない場合は、スタートメニューから「LAV Video Configuration」を起動し、Formats タブで対象コーデックが有効になっているか確認してください

### ハードウェアデコード（DXVA2）

メニューの「オプション → 詳細設定 → デコーダ → GPU 動画再生支援機能 (DXVA2) を有効にする」で設定します。
完全に有効にするにはデコーダ側の設定も必要です。

- **MS デコーダ:** DXVA2 に常時対応しています
- **LAV Filters:** 「LAV Video Configuration」の「Video Settings」タブ →「Hardware Acceleration」→「DXVA2 (native)」を選択し、プレイヤーを再構築してください

### 64bit 版（PCRPlayer64.exe）

処理内容は 32bit 版と同じです。32bit 版は 32bit フィルタのみ、64bit 版は 64bit フィルタのみ使用できます。

---

## Developer Guide

### Build requirements

- Visual Studio 2022 with Visual C++
- [DirectX SDK (June 2010)](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
- [Boost C++ Libraries](https://www.boost.org/)

Boost のパスは環境変数 `BOOST_ROOT` で指定してください（例: `C:\boost_1_90_0`）。
ビルド前に `BOOST_ROOT` が設定されていない場合、インクルードおよびライブラリの解決に失敗します。

## License

GNU General Public License v3.0 — see [LICENSE.txt](LICENSE.txt)

Based on PCRPlayer by narayado (http://pecatv.s25.xrea.com/)
