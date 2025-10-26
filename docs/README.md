# VinylTimecodeCHOP - Documentation Index

このディレクトリには VinylTimecodeCHOP プラグインの包括的なドキュメントが含まれています。

This directory contains comprehensive documentation for the VinylTimecodeCHOP plugin.

---

## 📚 ドキュメント一覧 / Documentation Index

### クイックスタート / Quick Start

初めての方はこちらから:
- **[QUICKSTART.md](../QUICKSTART.md)** - 最速で動作させるための手順
  - 動作確認済みの成功事例
  - 5分でセットアップ完了
  - Pioneer RekordBox DVS での設定例

### プロジェクト概要 / Project Overview

- **[README.md](../README.md)** - プロジェクトの全体像
  - 機能説明
  - 動作確認済み構成
  - 対応フォーマット一覧
  - ライセンス情報

### トラブルシューティング / Troubleshooting

問題が発生した場合:
- **[TROUBLESHOOTING.md](../TROUBLESHOOTING.md)** - 包括的なトラブルシューティングガイド
  - Info CHOP による診断フローチャート
  - 問題 A: プラグインが初期化されない
  - 問題 B: サンプルレートの不一致
  - 問題 C: タイムコードが検出されない
  - 問題 D: 品質が低い
  - 問題 E: 位置が更新されない
  - FAQ セクション

### ビルド手順 / Build Instructions

ソースからビルドする場合:
- **[BUILD.md](../BUILD.md)** - ビルド手順の詳細
  - macOS / Windows 対応
  - 必要な依存関係
  - CMake ビルド手順
  - トラブルシューティング

### インストール / Installation

- **[INSTALL_INSTRUCTIONS.txt](../INSTALL_INSTRUCTIONS.txt)** - インストール手順

### バージョン履歴 / Changelog

- **[CHANGELOG.md](../CHANGELOG.md)** - バージョン履歴と変更内容
  - v1.1.0: position = -1 問題を解決
  - v1.0.0: 初回リリース

### 既知の問題 / Known Issues

- **[issues.md](issues.md)** - 既知の問題と対応状況
  - Issue #1: position が -1 になる問題 [解決済み]

### 技術詳細 / Technical Details

- **[success-config.md](success-config.md)** - 成功構成の詳細記録
  - ハードウェア構成
  - ソフトウェア設定
  - 検証手順
  - パフォーマンス特性

### チュートリアル / Tutorials

- **[video-sync-tutorial.md](video-sync-tutorial.md)** - ビデオ同期チュートリアル ✨ NEW
  - バイナルレコードで動画を制御する方法
  - 再生・停止・逆再生の実装
  - TouchDesigner ネットワーク構成
  - トラブルシューティングとヒント

---

## ✅ 動作確認済み構成 / Verified Working Configuration

このプラグインは以下の構成で正常に動作することが確認されています:

```
Hardware:        Native Instruments Interface 2
Vinyl:           Pioneer RekordBox DVS Side A
Sample Rate:     44100 Hz
Switch Setting:  LINE (NOT PHONO)
TouchDesigner:   2023.11760
macOS:           Apple Silicon / Intel Universal Binary

Success Results:
  position:        292652.53
  pitch:           1.0
  quality:         1.40
  initialized:     1
  sample_rate:     44100
  format_index:    7 (Pioneer RekordBox Side A)
  valid_counter:   1639
  timecode_ticker: 7
```

この構成の詳細は [QUICKSTART.md](../QUICKSTART.md) を参照してください。

---

## 🎯 推奨読書順序 / Recommended Reading Order

### 初めて使用する方:

1. **[README.md](../README.md)** - プロジェクト概要を理解
2. **[QUICKSTART.md](../QUICKSTART.md)** - セットアップ手順に従う
3. **[TROUBLESHOOTING.md](../TROUBLESHOOTING.md)** - 問題が発生した場合に参照

### ソースからビルドする方:

1. **[BUILD.md](../BUILD.md)** - ビルド手順を確認
2. **[README.md](../README.md)** - プロジェクト構造を理解
3. **[QUICKSTART.md](../QUICKSTART.md)** - ビルド後のテスト方法

### 開発者の方:

1. **[README.md](../README.md)** - プロジェクト全体像
2. **[BUILD.md](../BUILD.md)** - 開発環境のセットアップ
3. **[TROUBLESHOOTING.md](../TROUBLESHOOTING.md)** - デバッグ手法の理解
4. ソースコード: `src/VinylTimecodeCHOP/`
5. xwax ライブラリ: `lib/xwax/`

---

## 🔍 重要な技術情報 / Important Technical Information

### 対応タイムコードフォーマット

| Index | Format | Bits | Frequency | Vinyl |
|-------|--------|------|-----------|-------|
| 0 | Serato 2nd Ed. Side A | 20-bit | 1000 Hz | Serato CV02 |
| 1 | Serato 2nd Ed. Side B | 20-bit | 1000 Hz | Serato CV02 |
| 2 | Serato CD | 20-bit | 1000 Hz | Serato CD |
| 3 | Traktor Scratch Side A | 23-bit | 2000 Hz | Traktor Scratch |
| 4 | Traktor Scratch Side B | 23-bit | 2000 Hz | Traktor Scratch |
| 5 | MixVibes DVS V2 | 20-bit | 1200 Hz | MixVibes |
| 6 | MixVibes 7inch | 20-bit | 1200 Hz | MixVibes |
| 7 | **Pioneer RekordBox Side A** | 20-bit | 1000 Hz | **Pioneer** ✅ |
| 8 | Pioneer RekordBox Side B | 20-bit | 1000 Hz | Pioneer |

**注意**: Traktor MK2 バイナル (110-bit LFSR) は現在サポートされていません。

### Info CHOP チャンネル

VinylTimecode CHOP は以下の 8 つの診断チャンネルを提供します:

| Channel | Description | Expected Value |
|---------|-------------|----------------|
| `initialized` | Timecoder initialized | 1 |
| `sample_rate` | Sample rate in Hz | 44100 |
| `format_index` | Selected timecode format | 0-8 |
| `valid_counter` | Valid decode count | > 0 when detecting |
| `timecode_ticker` | Internal timecode ticker | Incrementing value |
| `position` | Current position | Sample number |
| `pitch` | Current pitch | 0.5-2.0 (1.0 = normal) |
| `quality` | Signal quality | > 1.0 |

これらのチャンネルは問題診断に非常に有用です。詳細は [TROUBLESHOOTING.md](../TROUBLESHOOTING.md) を参照してください。

---

## 📁 プロジェクト構造 / Project Structure

```
touchdesignerOP/
├── README.md                    # プロジェクト概要
├── QUICKSTART.md                # クイックスタートガイド
├── TROUBLESHOOTING.md           # トラブルシューティング
├── BUILD.md                     # ビルド手順
├── INSTALL_INSTRUCTIONS.txt     # インストール手順
├── CMakeLists.txt               # ビルド設定
├── .gitignore                   # Git 除外設定 (ドキュメント保護)
│
├── docs/                        # ドキュメントディレクトリ
│   ├── README.md               # このファイル
│   └── images/                 # スクリーンショット用
│
├── src/
│   └── VinylTimecodeCHOP/      # プラグインソースコード
│       ├── VinylTimecodeCHOP.cpp
│       └── VinylTimecodeCHOP.h
│
├── lib/
│   └── xwax/                   # Mixxx 2.5.3 からの xwax ライブラリ
│
└── examples/
    └── Plugins/
        └── VinylTimecodeCHOP.plugin/  # ビルド済みプラグイン
```

---

## 🛠️ 重要な実装の詳細 / Critical Implementation Details

このプラグインの成功は以下の重要な修正に基づいています:

### 1. メニューパラメータの処理

```cpp
// 正しい実装 (Correct):
int formatIndex = inputs->getParInt("Format");
const char* formatName = TIMECODE_FORMATS[formatIndex];

// 誤った実装 (Wrong):
// const char* formatName = inputs->getParString("Format");
```

TouchDesigner のメニューパラメータは**整数インデックス**を返します。

### 2. シンボルエクスポート

CMakeLists.txt から `-fvisibility=hidden` フラグを削除する必要がありました:

```cmake
# 削除済み (Removed):
# -fvisibility=hidden
```

このフラグが原因でプラグインのエントリーポイントがエクスポートされませんでした。

### 3. 名前空間の修正

```cpp
using namespace TD;

extern "C" {
    DLLEXPORT void FillCHOPPluginInfo(TD::CHOP_PluginInfo *info)
    DLLEXPORT TD::CHOP_CPlusPlusBase* CreateCHOPInstance(const TD::OP_NodeInfo* info)
}
```

### 4. サンプルレート

最低 **44100 Hz** が必要です。TouchDesigner のデフォルト 60 Hz では動作しません。

---

## 📞 サポート / Support

### 問題が解決しない場合:

1. **[TROUBLESHOOTING.md](../TROUBLESHOOTING.md)** の診断フローチャートを確認
2. Info CHOP の全チャンネル値を確認
3. 以下の情報を収集:
   - TouchDesigner バージョン
   - macOS / Windows バージョン
   - オーディオインターフェース名
   - 使用しているバイナル
   - Info CHOP の全チャンネル値
   - Audio Device In CHOP の設定

### GitHub Issues テンプレート:

```markdown
## 環境
- TouchDesigner バージョン:
- macOS / Windows バージョン:
- オーディオインターフェース:
- バイナル:

## Info CHOP の値
- initialized:
- sample_rate:
- format_index:
- valid_counter:
- timecode_ticker:
- position:
- pitch:
- quality:

## Audio Device In CHOP の設定
- Rate:
- Time Slice:
- Input Names:
- Num Channels:

## 症状
(何が起きているか)

## 試した解決策
(試した手順)
```

---

## 📖 参考資料 / References

- [Mixxx Manual - Vinyl Control](https://manual.mixxx.org/2.5/ja/chapters/vinyl_control.html)
- [TouchDesigner C++ CHOP Documentation](https://docs.derivative.ca/Write_a_CPlusPlus_CHOP)
- [xwax Project](https://www.xwax.org/)
- [Native Instruments Interface 2](https://www.native-instruments.com/en/products/traktor/dj-audio-interfaces/traktor-audio-2/)

---

## 📄 ライセンス / License

このプロジェクトは GPL v3 ライセンスのもとで公開されています。
詳細は [README.md](../README.md) を参照してください。

---

**ドキュメントの保護について:**

このプロジェクトの `.gitignore` ファイルは、すべてのドキュメントファイルと動作確認済みプラグインを保護するように設定されています。これらのファイルは Git リポジトリから除外されることはありません。

---

**Happy Scratching!** 🎵🎧
