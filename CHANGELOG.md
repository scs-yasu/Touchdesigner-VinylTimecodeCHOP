# Changelog

VinylTimecodeCHOP の変更履歴を記録します。

---

## [1.2.0] - 2024-10-26

### 🐛 Fixed

**Issue #2: 逆回し時の position が飛び飛びになる問題を解決**

- 逆回し時に position が大きく飛び飛びになる問題を修正
- pitch ベースの補間により滑らかな position 変化を実現
- raw_position が -1 の間も pitch に基づいて position を推定

**実装内容**:
- `myLastNumSamples` メンバー変数を追加
- raw_position が -1 の場合、pitch × サンプル数で position を更新
- pitch の絶対値が 0.01 以上の場合のみ補間を実行

**補間ロジック**:
```cpp
if (myRawPosition >= 0) {
    myPosition = static_cast<double>(myRawPosition);
    myLastValidPosition = myPosition;
} else if (std::abs(myPitch) > 0.01) {
    // pitch ベースで position を推定
    double positionChange = myPitch * numSamples;
    myPosition = myLastValidPosition + positionChange;
    myLastValidPosition = myPosition;
} else {
    // 停止時は最後の値を保持
    myPosition = myLastValidPosition;
}
```

### ✨ Improved

**逆回し時の動作改善**:
- ✅ **逆回し時**: position が滑らかに減少する
- ✅ **通常再生時**: position が滑らかに増加する（引き続き正常）
- ✅ **停止時**: position が固定される（Issue #1 で解決済み、引き続き正常）

**pitch の活用**:
- pitch が負の場合、position は減少
- pitch が正の場合、position は増加
- pitch が 0 付近の場合、position は固定

### 📝 Documentation

- `docs/issues.md` に Issue #2 を追加
- `CHANGELOG.md` に v1.2.0 を追加

---

## [1.1.0] - 2024-10-26

### 🐛 Fixed

**Issue #1: position が -1 になる問題を解決**

- レコード停止時に position が -1 になる問題を修正
- 反対回し（逆再生）時に position が -1 になる問題を修正
- 最後の有効な position を保持するロジックを実装

**詳細**:
- `myLastValidPosition` メンバー変数を追加
- `myRawPosition` メンバー変数を追加（デバッグ用）
- `timecoder_get_position()` の戻り値 `-1` を適切に処理
- `-1` の場合、最後の有効な position を使用

**期待される動作**:
- **レコード停止時**: position が最後の値で固定される ✅
- **反対回し時**: position が減少していく（正の値のまま）✅
- **通常再生時**: position が増加し続ける ✅

### ✨ Added

**Info CHOP の拡張**:
- `raw_position` チャンネルを追加（9つ目のチャンネル）
- 生の `timecoder_get_position()` の戻り値を表示
- デバッグが容易に

**Info CHOP チャンネル一覧**:
1. `position` - 修正後の position（常に >= 0）
2. `pitch` - ピッチ
3. `quality` - 品質
4. `initialized` - 初期化状態
5. `sample_rate` - サンプルレート
6. `format_index` - フォーマットインデックス
7. `valid_counter` - 有効カウンタ
8. `timecode_ticker` - タイムコードティッカー
9. **`raw_position`** - **生の position（-1 の可能性あり）** ← NEW

### 📝 Documentation

- `docs/issues.md` を作成（問題追跡）
- `CHANGELOG.md` を作成（バージョン履歴）
- 修正内容をドキュメント化

### 🔍 Changed

**修正ファイル**:
- `src/VinylTimecodeCHOP/VinylTimecodeCHOP.h`
  - `myLastValidPosition` を追加
  - `myRawPosition` を追加

- `src/VinylTimecodeCHOP/VinylTimecodeCHOP.cpp`
  - コンストラクタで新しいメンバーを初期化
  - `processAudioSamples()` で `-1` のチェックと処理を追加
  - `getNumInfoCHOPChans()` を 9 に更新
  - `getInfoCHOPChan()` に `raw_position` を追加

---

## [1.0.0] - 2024-10-25

### 🎉 Initial Release

**動作確認済み構成**:
- ハードウェア: Native Instruments Interface 2
- バイナル: Pioneer RekordBox DVS Side A
- サンプルレート: 44100 Hz
- スイッチ: LINE (NOT PHONO)
- TouchDesigner: 2023.11760
- macOS: Apple Silicon / Intel Universal Binary

**成功パラメータ**:
```
position:        292652.53
pitch:           1.0
quality:         1.40
initialized:     1
sample_rate:     44100
format_index:    7 (Pioneer RekordBox Side A)
valid_counter:   1639
timecode_ticker: 7
```

### ✨ Features

**対応タイムコードフォーマット**:
- Serato 2nd Ed. Side A/B
- Serato CD
- Traktor Scratch Side A/B
- MixVibes DVS V2
- MixVibes 7inch
- **Pioneer RekordBox Side A/B** ✅

**Info CHOP チャンネル** (8チャンネル):
1. `position` - タイムコード位置
2. `pitch` - 再生速度
3. `quality` - 信号品質
4. `initialized` - 初期化状態
5. `sample_rate` - サンプルレート
6. `format_index` - フォーマットインデックス
7. `valid_counter` - 有効カウンタ
8. `timecode_ticker` - タイムコードティッカー

### 🐛 Known Issues

- **Issue #1**: レコード停止時と反対回し時に position が -1 になる
  - v1.1.0 で修正済み

### 🔧 Technical Details

**重要な修正**:
1. メニューパラメータ処理: `getParString()` → `getParInt()`
2. シンボルエクスポート: `-fvisibility=hidden` フラグを削除
3. 名前空間: `using namespace TD;` を追加

**ビルド構成**:
- コンパイラ: Apple Clang 15.0.0
- C++ 標準: C++14
- アーキテクチャ: Universal Binary (arm64 + x86_64)
- 最適化: -O2

**ライブラリ**:
- xwax 1.6 (Mixxx 2.5.3 から)
- GPL v3 ライセンス

### 📝 Documentation

**作成されたドキュメント**:
- `README.md` - プロジェクト概要
- `QUICKSTART.md` - クイックスタートガイド
- `TROUBLESHOOTING.md` - トラブルシューティングガイド
- `BUILD.md` - ビルド手順
- `INSTALL_INSTRUCTIONS.txt` - インストール手順
- `docs/README.md` - ドキュメントインデックス
- `docs/success-config.md` - 成功構成の詳細
- `.gitignore` - Git 除外設定（ドキュメント保護）

---

## バージョニング

このプロジェクトは [Semantic Versioning](https://semver.org/) に従います:

- **MAJOR**: 互換性のない API 変更
- **MINOR**: 後方互換性のある機能追加
- **PATCH**: 後方互換性のあるバグ修正

---

## リンク

- [GitHub Issues](https://github.com/yourusername/touchdesignerOP/issues)
- [Mixxx xwax](https://github.com/mixxxdj/mixxx)
- [TouchDesigner](https://derivative.ca/)

---

**最終更新**: 2024-10-26
