# VinylTimecodeCHOP - 成功構成の詳細

このドキュメントは、VinylTimecodeCHOP が正常に動作した構成の詳細な記録です。

---

## 🎉 成功事例の概要

**日時**: 2024年10月25日  
**ステータス**: ✅ 完全動作確認済み

### スクリーンショット結果

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

---

## 🔧 ハードウェア構成

### オーディオインターフェース

**製品名**: Native Instruments Interface 2  
**接続**: USB  
**設定**:
- **入力スイッチ**: LINE (❗ PHONO ではない)
- **入力チャンネル**: L/R (2チャンネル)
- **サンプルレート**: 44100 Hz
- **LED インジケータ**: 
  - 緑: 正常な接続
  - 赤: スイッチが LINE に設定されている
  - オレンジ: 信号が入力されている

### ターンテーブルとバイナル

**ターンテーブル**: Technics SL-1200 (または互換機)  
**バイナル**: Pioneer RekordBox DVS Side A  
**RPM**: 33 1/3  
**接続**: ターンテーブル → Interface 2 (LINE入力)

### 接続図

```
[ターンテーブル]
    ↓ RCA ケーブル (L/R)
[Interface 2] - スイッチ: LINE
    ↓ USB
[Mac] (macOS)
    ↓
[TouchDesigner]
    ↓
[VinylTimecode CHOP]
```

---

## 💻 ソフトウェア構成

### TouchDesigner

**バージョン**: 2023.11760  
**プラットフォーム**: macOS (Apple Silicon / Intel Universal Binary)

### macOS オーディオ設定

**システム設定 > サウンド > 入力**:
- **選択デバイス**: Native Instruments Interface 2
- **入力レベル**: 中程度に設定
- **サンプルレート**: 44100 Hz (Audio MIDI Setup で確認)

### プラグイン配置

**場所**: `~/Desktop/project1/Plugins/VinylTimecodeCHOP.plugin/`

**配置コマンド**:
```bash
PROJECT_PATH="/path/to/your/project"
mkdir -p "$PROJECT_PATH/Plugins"
cp -R ./examples/Plugins/VinylTimecodeCHOP.plugin "$PROJECT_PATH/Plugins/"
xattr -cr "$PROJECT_PATH/Plugins/VinylTimecodeCHOP.plugin"
```

---

## 🎛️ TouchDesigner ネットワーク構成

### Audio Device In CHOP

**パラメータ設定**:
```
Driver: Core Audio
Device: Native Instruments Interface 2
Rate: 44100 Hz
Sample Rate: 44100
Time Slice: On
Input Names: 0 1
Active Channels: 0 1
Num Channels: 2
```

### VinylTimecode CHOP

**パラメータ設定**:
```
Format: Pioneer RekordBox Side A (index 7)
Input: audioDevin1 (Audio Device In CHOP)

Common:
  Cook Every Frame: Off (重要!)
  Time Slice: Off
```

**接続**:
```
[audioDevin1 (Audio Device In CHOP)]
    ↓
[vinyltimecode1 (VinylTimecode CHOP)]
    ↓ 出力チャンネル:
    - position
    - pitch
    - quality
```

### Info CHOP (診断用)

VinylTimecode CHOP 上で右クリック → "Info CHOP" を選択

**確認すべき値**:
```
initialized:     1      ✅ 初期化成功
sample_rate:     44100  ✅ 正しいサンプルレート
format_index:    7      ✅ Pioneer RekordBox Side A
valid_counter:   > 0    ✅ タイムコード検出中
timecode_ticker: > 0    ✅ タイムコードティッカー動作中
position:        > 0    ✅ 位置更新中
pitch:           ~1.0   ✅ 正常な再生速度
quality:         > 1.0  ✅ 良好な品質
```

---

## 📊 期待される動作

### 正常動作時の挙動

1. **バイナルを再生すると**:
   - `position` が増加し続ける (292652.53 など)
   - `pitch` が 1.0 付近で安定
   - `quality` が 1.0 以上
   - `valid_counter` が増加し続ける

2. **バイナルを逆再生すると**:
   - `position` が減少
   - `pitch` が負の値

3. **バイナルを速く回すと**:
   - `pitch` が 1.0 より大きくなる (例: 1.5)

4. **バイナルをゆっくり回すと**:
   - `pitch` が 1.0 より小さくなる (例: 0.5)

5. **バイナルを止めると**:
   - `position` が最後の値で固定
   - `pitch` が 0.0 に近づく
   - `valid_counter` の増加が止まる

### 数値の意味

**position**:
- サンプル数での位置
- 44100 Hz の場合、44100 = 1秒
- 292652.53 サンプル ≈ 6.6 秒の位置

**pitch**:
- 再生速度の倍率
- 1.0 = 通常速度 (33 1/3 RPM)
- 2.0 = 2倍速
- 0.5 = 半速
- -1.0 = 逆再生

**quality**:
- 信号品質の指標
- 1.0 以上が良好
- 10 以上は非常に良好
- `valid_counter` に基づいて計算される

---

## 🔍 重要な実装の詳細

### 1. メニューパラメータの処理

**成功のカギとなった修正**:

```cpp
// ❌ 誤った実装 (動作しない):
const char* formatName = inputs->getParString("Format");

// ✅ 正しい実装 (動作する):
int formatIndex = inputs->getParInt("Format");
const char* formatName = TIMECODE_FORMATS[formatIndex];
```

**理由**:
TouchDesigner のメニューパラメータは文字列ではなく**整数インデックス**を返します。

### 2. シンボルエクスポートの修正

**CMakeLists.txt の修正**:

```cmake
# ❌ 削除した設定 (プラグインがロードされない):
-fvisibility=hidden

# ✅ 現在の設定 (プラグインがロードされる):
target_compile_options(VinylTimecodeCHOP PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
)
```

**検証方法**:
```bash
nm -gU VinylTimecodeCHOP.plugin/Contents/MacOS/VinylTimecodeCHOP | grep Fill
```

**期待される出力**:
```
00000000000014a0 T _FillCHOPPluginInfo
```

`T` (グローバルシンボル) であることが重要。`t` (ローカル) だとロード失敗。

### 3. 名前空間の修正

**VinylTimecodeCHOP.h**:

```cpp
#include "CHOP_CPlusPlusBase.h"

using namespace TD;  // ← これが重要

class VinylTimecodeCHOP : public CHOP_CPlusPlusBase
{
    // ...
};
```

**VinylTimecodeCHOP.cpp**:

```cpp
extern "C"
{
    DLLEXPORT void FillCHOPPluginInfo(TD::CHOP_PluginInfo *info)
    {
        // TD:: プレフィックスが必要
    }
}
```

### 4. サンプルレートの要件

**Audio Device In CHOP の Rate パラメータ**:
- ✅ 44100 Hz: 動作する
- ✅ 48000 Hz: 動作する (ただし 44100 を推奨)
- ❌ 60 Hz: 動作しない (TouchDesigner のデフォルト)

**確認方法**:
```python
n = op('audioDevin1')
print(f"Sample Rate: {n.sampleRate}")  # 44100.0 であること
```

---

## 🧪 検証手順

### 1. 基本動作確認

```
1. バイナルを再生
2. Info CHOP で以下を確認:
   - initialized = 1
   - sample_rate = 44100
   - format_index = 7
   - valid_counter > 0 (増加中)
   - position > 0 (増加中)
   - quality > 1.0
```

### 2. ピッチテスト

```
1. バイナルを通常速度で再生
   → pitch ≈ 1.0

2. バイナルを速く回す
   → pitch > 1.0 (例: 1.5)

3. バイナルをゆっくり回す
   → pitch < 1.0 (例: 0.5)

4. バイナルを逆再生
   → pitch < 0 (例: -1.0)
```

### 3. ポジショントラッキングテスト

```
1. バイナルの開始位置にセット
2. position の値を記録 (例: A = 100000)
3. 1回転させる
4. position の値を記録 (例: B = 105000)
5. 差分を計算: B - A ≈ 5000 サンプル
```

### 4. 品質テスト

```
良好な品質 (quality > 1.0) が維持されることを確認:
- バイナルが清潔
- スタイラスが清潔
- 接続ケーブルがしっかり接続
- Interface 2 のスイッチが LINE
- 信号レベルが適切 (0.02 ~ 0.10)
```

---

## 🚨 よくある問題と解決策

### position = 0 のまま

**チェックリスト**:
1. ✅ `initialized = 1` か?
2. ✅ `sample_rate = 44100` か?
3. ✅ `format_index = 7` (Pioneer) か?
4. ✅ Interface 2 のスイッチが LINE か?
5. ✅ Audio Device In の Rate が 44100 か?

### quality が低い (< 1.0)

**チェックリスト**:
1. ✅ バイナルが清潔か?
2. ✅ スタイラスが清潔か?
3. ✅ 正しい面 (Side A) を使用しているか?
4. ✅ Interface 2 の入力スイッチが LINE か?

### TouchDesigner が応答しない

**チェックリスト**:
1. ✅ VinylTimecode CHOP の "Cook Every Frame" が Off か?
2. ✅ Audio Device In の "Time Slice" が On か?
3. ✅ Perform Mode が有効か?

---

## 📈 パフォーマンス特性

### CPU 使用率

**通常動作時**:
- VinylTimecode CHOP: < 1% CPU
- Audio Device In CHOP: < 2% CPU
- 合計: < 5% CPU (M1 Mac で測定)

### レイテンシ

**典型的な値**:
- Audio Device In の Buffer Size: 512 サンプル
- レイテンシ: ~11.6 ms (512 / 44100)
- 実用上問題ないレベル

### メモリ使用量

**典型的な値**:
- プラグイン自体: < 1 MB
- オーディオバッファ: ~200 KB

---

## 🔐 セキュリティとアクセス許可

### macOS セキュリティ属性

**プラグイン配置後に必須**:
```bash
xattr -cr /path/to/Plugins/VinylTimecodeCHOP.plugin
```

**理由**:
macOS は外部からダウンロードしたファイルに隔離属性 (quarantine) を付与します。これを削除しないとプラグインがロードされません。

**確認方法**:
```bash
xattr -l /path/to/Plugins/VinylTimecodeCHOP.plugin
```

**期待される出力**:
```
(何も表示されないのが正常)
```

### TouchDesigner のアクセス許可

**システム設定 > プライバシーとセキュリティ**:
- ✅ TouchDesigner にマイクへのアクセスを許可
- ✅ TouchDesigner に入力監視を許可 (必要に応じて)

---

## 📝 開発ノート

### ビルド構成

**コンパイラ**: Apple Clang 15.0.0  
**C++ 標準**: C++14  
**アーキテクチャ**: Universal Binary (arm64 + x86_64)  
**最適化**: -O2

**重要なコンパイラフラグ**:
```cmake
-Wall                    # すべての警告を表示
-Wextra                  # 追加の警告
-Wno-unused-parameter    # 未使用パラメータ警告を無視
```

**削除したフラグ**:
```cmake
# -fvisibility=hidden    # ← これを削除することが重要!
```

### xwax ライブラリ

**ソース**: Mixxx 2.5.3  
**バージョン**: xwax 1.6 ベース  
**ライセンス**: GPL v3

**含まれるファイル**:
- `timecoder.c` / `timecoder.h` - メインのタイムコーダー実装
- `lut.c` / `lut.h` - ルックアップテーブル
- `pitch_kalman.c` / `pitch_kalman.h` - カルマンフィルタによるピッチ推定
- `pitch.h` - ピッチ推定のインターフェース
- `debug.h` - デバッグマクロ

---

## 🎯 成功要因の分析

### 成功のカギとなった3つの修正

1. **メニューパラメータの処理修正**
   - `getParString()` → `getParInt()`
   - この修正により正しいタイムコードフォーマットが選択されるようになった

2. **シンボルエクスポートの修正**
   - `-fvisibility=hidden` の削除
   - この修正によりプラグインがロードされるようになった

3. **名前空間の修正**
   - `using namespace TD;` と `TD::` プレフィックス
   - この修正によりコンパイルが通るようになった

### 動作確認までの道のり

```
1. 最初のビルド → コンパイルエラー (名前空間の問題)
2. 名前空間修正 → ビルド成功、しかしシンボル未エクスポート
3. visibility修正 → プラグインロード成功、しかし position = 0
4. パラメータ修正 → ✅ 完全動作!
```

---

## 📅 バージョン履歴

### v1.0.0 (2024-10-25) - 初回成功

**動作確認**:
- ✅ Pioneer RekordBox DVS Side A で完全動作
- ✅ Interface 2 で動作確認
- ✅ macOS Universal Binary
- ✅ TouchDesigner 2023.11760 で動作確認

**既知の制限**:
- ❌ Traktor MK2 バイナル (110-bit LFSR) は未対応

---

## 🔗 関連ドキュメント

- [QUICKSTART.md](../QUICKSTART.md) - クイックスタートガイド
- [TROUBLESHOOTING.md](../TROUBLESHOOTING.md) - トラブルシューティング
- [BUILD.md](../BUILD.md) - ビルド手順
- [README.md](../README.md) - プロジェクト概要

---

**このドキュメントは成功構成の完全な記録です。**  
**この設定を変更する前に、必ずバックアップを取ってください。**

Happy Scratching! 🎵🎧
