# Plan: BaseClasses をリポジトリに取り込む

## Context

現在の問題点：
- **LibraryPath**: `C:\Program Files\Microsoft SDKs\Windows\v7.1\Samples\multimedia\directshow\baseclasses\lib2015\x86(or x64)` をハードコード
- **IncludePath**: システムのデフォルトパス（user.props 経由）から `<streams.h>` をインクルード
- **リンク**: `GraphManager.h` の `#pragma comment(lib, "strmbase.lib" / "strmbasd.lib")` で指定

別PCでクローンしてもビルドできない状態。BaseClasses のソースをリポジトリに取り込み、プロジェクト参照でビルドする構成に変更する。

**BaseClasses ソースの取得元**:
GitHub `microsoft/Windows-classic-samples` リポジトリ。
チェックアウト済みパス: `D:\Workspace\Windows-classic-samples\Samples\Win7Samples\multimedia\directshow\baseclasses`

含まれるファイル（ソース一覧）:
```
streams.h, amextra.cpp/h, amfilter.cpp/h, amvideo.cpp, arithutil.cpp,
combase.cpp/h, cprop.cpp/h, ctlutil.cpp/h, ddmm.cpp/h, dllentry.cpp,
dllsetup.cpp/h, mtype.cpp/h, outputq.cpp/h, perflog.cpp/h,
pstream.cpp/h, pullpin.cpp/h, refclock.cpp/h, renbase.cpp/h,
schedule.cpp/h, seekpt.cpp/h, source.cpp/h, strmctl.cpp/h,
sysclock.cpp/h, transfrm.cpp/h, transip.cpp/h, videoctl.cpp/h,
vtrans.cpp/h, winctrl.cpp/h, winutil.cpp/h, wxdebug.cpp/h,
wxlist.cpp/h, wxutil.cpp/h, ...
```

※ 既存の `baseclasses.vcproj` は旧 VS2008 形式のため使用しない。新たに `strmbase.vcxproj` を作成する。

---

## 変更対象ファイル

| ファイル | 変更種別 |
|---------|---------|
| `external/baseclasses/` | 新規作成（ソースファイルのコピー） |
| `external/baseclasses/strmbase.vcxproj` | 新規作成（旧 .vcproj は使わず新規で作成） |
| `PCRPlayer.sln` | strmbase プロジェクト追加 |
| `PCRPlayer/PCRPlayer.vcxproj` | IncludePath追加・LibraryPath削除・ProjectReference追加 |
| `PCRPlayer/GraphManager.h` | `#pragma comment(lib, ...)` を削除 |

---

## 実施ステップ

### Step 1: BaseClasses ソースをコピー

チェックアウト済みソースから `external/baseclasses/` へコピーする。
`.vcproj`, `.sln` は不要なので除外する。

```
robocopy "D:\Workspace\Windows-classic-samples\Samples\Win7Samples\multimedia\directshow\baseclasses" "d:\Workspace\PCRPlayer\external\baseclasses" *.cpp *.h
```

---

### Step 2: strmbase.vcxproj を作成

`external/baseclasses/strmbase.vcxproj` を新規作成。

| 設定項目 | 値 |
|---------|---|
| ConfigurationType | StaticLibrary |
| PlatformToolset | v143（VS2022） |
| WindowsTargetPlatformVersion | 10.0.26100.0 |
| CharacterSet | MultiByte（BaseClasses は MBCS） |
| プリコンパイルヘッダー | 無効化（NotUsing）※ソースが多様なため |
| Debug 出力名 | strmbasd |
| Release 出力名 | strmbase |
| 構成 | Debug/Release × Win32/x64 |

**ソースファイル（ClCompile）**:
```
amextra.cpp, amfilter.cpp, amvideo.cpp, arithutil.cpp, combase.cpp,
cprop.cpp, ctlutil.cpp, ddmm.cpp, dllentry.cpp, dllsetup.cpp,
mtype.cpp, outputq.cpp, perflog.cpp, pstream.cpp, pullpin.cpp,
refclock.cpp, renbase.cpp, schedule.cpp, seekpt.cpp, source.cpp,
strmctl.cpp, sysclock.cpp, transfrm.cpp, transip.cpp, videoctl.cpp,
vtrans.cpp, winctrl.cpp, winutil.cpp, wxdebug.cpp, wxlist.cpp, wxutil.cpp
```

※ PCH は不使用（BaseClasses 内の各 .cpp が `streams.h` を直接インクルードするため、PCH 管理が複雑になる）。

---

### Step 3: PCRPlayer.sln を更新

strmbase.vcxproj をソリューションに追加する。

```
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "strmbase", "external\baseclasses\strmbase.vcxproj", "{新GUID}"
EndProject
```

GlobalSection(ProjectConfigurationPlatforms) に4構成分のエントリを追加：
```
{新GUID}.Debug|Win32.ActiveCfg = Debug|Win32
{新GUID}.Debug|Win32.Build.0 = Debug|Win32
{新GUID}.Debug|x64.ActiveCfg = Debug|x64
{新GUID}.Debug|x64.Build.0 = Debug|x64
{新GUID}.Release|Win32.ActiveCfg = Release|Win32
{新GUID}.Release|Win32.Build.0 = Release|Win32
{新GUID}.Release|x64.ActiveCfg = Release|x64
{新GUID}.Release|x64.Build.0 = Release|x64
```

---

### Step 4: PCRPlayer.vcxproj を更新

**4-A: PlatformToolset と WindowsTargetPlatformVersion を更新**

strmbase.vcxproj と ABI を合わせるため、PCRPlayer.vcxproj も同じ値に変更する。

```xml
<!-- 変更前（全4構成の Configuration PropertyGroup） -->
<PlatformToolset>v140_xp</PlatformToolset>
...
<WindowsTargetPlatformVersion>8.1</WindowsTargetPlatformVersion>

<!-- 変更後 -->
<PlatformToolset>v143</PlatformToolset>
...
<WindowsTargetPlatformVersion>10.0.26100.0</WindowsTargetPlatformVersion>
```

**4-B: IncludePath を更新（全4構成）**

```xml
<!-- 変更前 -->
<IncludePath>$(IncludePath)</IncludePath>

<!-- 変更後 -->
<IncludePath>$(SolutionDir)external\baseclasses;$(IncludePath)</IncludePath>
```

**4-C: LibraryPath を削除（全4構成）**

以下の行を削除：
```xml
<!-- 削除対象（x86の例、x64も同様） -->
<LibraryPath>C:\Program Files\Microsoft SDKs\Windows\v7.1\Samples\multimedia\directshow\baseclasses\lib2015\x86;$(LibraryPath)</LibraryPath>
```

**4-D: ProjectReference を追加**

```xml
<ItemGroup>
  <ProjectReference Include="..\external\baseclasses\strmbase.vcxproj">
    <Project>{新GUID}</Project>
  </ProjectReference>
</ItemGroup>
```

---

### Step 5: GraphManager.h を更新

Project Reference がリンクを自動解決するため、以下のブロックを削除する。

```cpp
// 削除（GraphManager.h L29-33）
#ifndef _DEBUG
#pragma comment(lib, "strmbase.lib")
#else
#pragma comment(lib, "strmbasd.lib")
#endif
```

---

## 注意事項

- **PCH について**: BaseClasses の各 `.cpp` は冒頭で `streams.h` をインクルードしている。PCH を有効にする場合は `dllentry.cpp` 等1ファイルを「作成」に設定し残りを「使用」にする必要があるが、手間がかかるため今回は無効化（NotUsing）を推奨。
- **CharacterSet**: BaseClasses は MBCS（MultiByte）。PCRPlayer 側は Unicode。strmbase.vcxproj は MultiByte で設定する。
- **ToolsetのABI互換性**: strmbase.vcxproj と PCRPlayer.vcxproj は両方 v143 / 10.0.26100.0 に揃えるため ABI の問題はない。v140_xp は VS2022 では利用不可のため両プロジェクトまとめて移行する。
- `<streams.h>` のインクルード解決: IncludePath 追加後は `$(SolutionDir)external\baseclasses\streams.h` が使われる。
- Project Reference を使うと MSBuild がビルド順序を自動解決し、strmbase が先にビルドされる。

---

## 検証方法

1. Visual Studio でソリューションを開く
2. ビルド → ソリューションのビルド（strmbase が先にビルドされることを確認）
3. Debug/Release × Win32/x64 の4構成すべてでビルドが通ることを確認
4. `C:\Program Files\...` 以下の lib ファイルなしでビルドできることを確認
