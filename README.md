# PCRPlayer (modified fork)

A modified fork of [PCRPlayer](http://pecatv.s25.xrea.com/) —
a Windows BBS viewer/poster for 2ch/Shitaraba style boards.

## Build requirements

- Visual Studio 2022 with Visual C++
- [DirectX SDK (June 2010)](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
- [Boost C++ Libraries](https://www.boost.org/)

Boost のパスは環境変数 `BOOST_ROOT` で指定してください（例: `C:\boost_1_90_0`）。
ビルド前に `BOOST_ROOT` が設定されていない場合、インクルードおよびライブラリの解決に失敗します。

## License

GNU General Public License v3.0 — see [LICENSE.txt](LICENSE.txt)

Based on PCRPlayer by narayado (http://pecatv.s25.xrea.com/)
