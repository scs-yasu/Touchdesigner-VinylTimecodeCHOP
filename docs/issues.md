# VinylTimecodeCHOP - 既知の問題と対応状況

このドキュメントは問題と対応状況を記録します。

---

## ✅ Issue #1: position が -1 になる問題 [解決済み]

**報告日**: 2024-10-26  
**解決日**: 2024-10-26  
**ステータス**: ✅ 解決済み  
**優先度**: 高

### 問題の詳細

position が以下の状況で `-1` になっていた:

1. **レコードを止めたとき**
   - 期待される動作: position が最後の値で固定される
   - 実際の動作: position が -1 になる ❌

2. **反対回し（逆再生）したとき**
   - 期待される動作: position が減少していく（正の値のまま）
   - 実際の動作: position が -1 になる ❌

### 原因の特定

**xwax ライブラリの仕様** (`lib/xwax/timecoder.c:726-746`):

```c
signed int timecoder_get_position(struct timecoder *tc, double *when)
{
    signed int r;

    // valid_counter が 24 以下の場合、-1 を返す
    if (tc->valid_counter <= VALID_BITS)
        return -1;  // ← 問題の原因

    r = lut_lookup(&tc->def->lut, tc->bitstream);
    if (r == -1)
        return -1;  // ルックアップテーブルで見つからない場合も -1
    
    // ...
    return r;
}
```

**コメント**: "Return: the known position of the timecode, or -1 if not known"

`-1` は「position が不明」を示す特別な値。以下の場合に返される：
1. `valid_counter <= 24` の場合（信号が弱い、停止時）
2. ルックアップテーブルで見つからない場合（逆再生時など）

### 解決策

**修正内容**:

1. **ヘッダーファイル** (`src/VinylTimecodeCHOP/VinylTimecodeCHOP.h`):
   - `myLastValidPosition` メンバーを追加
   - `myRawPosition` メンバーを追加（デバッグ用）

2. **実装ファイル** (`src/VinylTimecodeCHOP/VinylTimecodeCHOP.cpp`):

```cpp
// Get the latest values
myRawPosition = timecoder_get_position(myTimecoder, nullptr);

// Handle -1 (position unknown) by keeping the last valid position
if (myRawPosition >= 0) {
    // Valid position received, update both current and last valid
    myPosition = static_cast<double>(myRawPosition);
    myLastValidPosition = myPosition;
} else {
    // Position is -1 (unknown), keep the last valid position
    myPosition = myLastValidPosition;
}
```

3. **Info CHOP の拡張**:
   - 9つ目のチャンネル `raw_position` を追加
   - `-1` の検出とデバッグが容易に

### 期待される動作（修正後）

**レコード停止時**:
- `position`: 最後の有効な値で固定 ✅
- `raw_position`: -1
- `pitch`: 0.0 付近
- `valid_counter`: <= 24

**反対回し時**:
- `position`: 減少していく（正の値のまま）✅
- `raw_position`: 正の値または -1
- `pitch`: 負の値

**通常再生時**:
- `position`: 増加していく（例: 292652.53）✅
- `raw_position`: position と同じ
- `pitch`: 1.0 付近

### Info CHOP チャンネル（更新後）

修正により、9つのチャンネルを提供：

| Channel | Description | Expected Value |
|---------|-------------|----------------|
| `position` | **修正後の position** | 常に >= 0（最後の有効値を保持） |
| `pitch` | Current pitch | 0.5-2.0 (1.0 = normal) |
| `quality` | Signal quality | > 1.0 |
| `initialized` | Timecoder initialized | 1 |
| `sample_rate` | Sample rate in Hz | 44100 |
| `format_index` | Selected timecode format | 0-8 |
| `valid_counter` | Valid decode count | > 0 when detecting |
| `timecode_ticker` | Internal timecode ticker | Incrementing value |
| **`raw_position`** | **生の position（-1 の可能性あり）** | **>= 0 または -1** |

### テスト方法

1. **通常再生テスト**:
   ```
   バイナルを再生
   → position が増加し続ける
   → raw_position == position
   → valid_counter > 24
   ```

2. **停止テスト**:
   ```
   バイナルを再生 → 停止
   → position が最後の値で固定される ✅
   → raw_position == -1
   → valid_counter <= 24
   ```

3. **逆再生テスト**:
   ```
   バイナルを逆回し
   → position が減少していく ✅
   → raw_position が正の値または -1
   → pitch が負の値
   ```

### 修正ファイル

- ✅ `src/VinylTimecodeCHOP/VinylTimecodeCHOP.h`
- ✅ `src/VinylTimecodeCHOP/VinylTimecodeCHOP.cpp`
- ✅ `examples/Plugins/VinylTimecodeCHOP.plugin/` (再ビルド)

### 参考

- [xwax timecoder.c](../lib/xwax/timecoder.c) - `timecoder_get_position()` 実装
- [Mixxx vinylcontrolxwax.cpp](../mixxx-2.5.3/src/vinylcontrol/vinylcontrolxwax.cpp) - Mixxx での使用例

### バージョン

- **v1.0.0**: 初回成功（position = -1 問題あり）
- **v1.1.0**: position = -1 問題を解決 ✅

---

## ✅ Issue #2: 逆回し時の position が飛び飛びになる問題 [解決済み]

**報告日**: 2024-10-26
**解決日**: 2024-10-26
**ステータス**: ✅ 解決済み
**優先度**: 中

### 問題の詳細

**v1.1.0 での動作確認**:
- ✅ **レコード停止時**: position が正常に固定される（Issue #1 で解決）
- ⚠️ **逆回し時**: position は戻るが、数値が大きく飛び飛びになる

**ユーザーからの報告**:
> 「逆に回した時に、確かに position が元に戻るのですが、その数値が大きく飛び飛びな感じで、もう少し滑らかに戻って欲しい」

### 期待される動作

**逆回し時**:
- position が**滑らかに**減少していく
- 飛び飛びにならず、連続的に変化する
- pitch が負の値の時に自然な動きになる

### 推測される原因

1. **raw_position が -1 を頻繁に返す**
   - 逆再生時にルックアップテーブルで見つからない
   - -1 の場合、最後の有効値を保持するため、position が変化しない
   - 次に有効な値が来た時に大きくジャンプする

2. **現在のロジックの問題**:
   ```cpp
   if (myRawPosition >= 0) {
       myPosition = static_cast<double>(myRawPosition);
       myLastValidPosition = myPosition;
   } else {
       // -1 の時は最後の値を保持（停止時には正しいが、逆回し時は不適切）
       myPosition = myLastValidPosition;
   }
   ```

3. **pitch 情報を活用していない**
   - pitch が負の場合、position は減少しているはず
   - -1 の間も pitch に基づいて position を補間すべき

### 調査項目

- [ ] 逆回し時の `raw_position` の値を記録
- [ ] -1 が返される頻度を確認
- [ ] pitch の値と position の変化の関係を分析
- [ ] Mixxx での逆回し処理を参考にする

### 対応方針

**アプローチ 1: pitch ベースの補間**
- pitch が負の場合、-1 の間も position を減少させる
- pitch の値に基づいて position を推定

**アプローチ 2: 線形補間**
- 前回の有効な position と今回の有効な position の間を補間
- -1 の期間を考慮して滑らかに変化させる

**アプローチ 3: カルマンフィルタ**
- xwax に含まれる pitch_kalman を活用
- position の推定精度を向上

### Info CHOP での確認事項

逆回し時に以下を確認:
```
raw_position:    -1 が頻繁に出る？
position:        飛び飛びに変化？
pitch:           負の値で安定？
valid_counter:   どの程度の値？
timecode_ticker: 増加している？
```

### 参考

- [Issue #1](# Issue #1) - position = -1 の基本的な処理
- pitch_kalman.c - カルマンフィルタによる推定

### 解決策

**pitch ベースの補間を実装** (`src/VinylTimecodeCHOP/VinylTimecodeCHOP.cpp`):

```cpp
// Get the latest values
myRawPosition = timecoder_get_position(myTimecoder, nullptr);
myPitch = timecoder_get_pitch(myTimecoder);

// Handle -1 (position unknown) with pitch-based interpolation
if (myRawPosition >= 0) {
    // Valid position received, update both current and last valid
    myPosition = static_cast<double>(myRawPosition);
    myLastValidPosition = myPosition;
} else {
    // Position is -1 (unknown)
    // Use pitch-based interpolation for smooth movement
    if (myLastNumSamples > 0 && std::abs(myPitch) > 0.01) {
        // Estimate position change based on pitch and samples
        // pitch is in relative speed (1.0 = normal, -1.0 = reverse at normal speed)
        double positionChange = myPitch * numSamples;
        myPosition = myLastValidPosition + positionChange;

        // Update last valid position for next iteration
        myLastValidPosition = myPosition;
    } else {
        // No pitch information or stopped, keep the last valid position
        myPosition = myLastValidPosition;
    }
}

// Store sample count for next iteration
myLastNumSamples = numSamples;
```

**追加メンバー変数**:
- `myLastNumSamples`: サンプル数を保存（次の補間で使用）

**補間ロジックの仕組み**:
1. raw_position が有効（>= 0）: そのまま使用
2. raw_position が -1 で pitch がある: pitch × サンプル数で推定
3. raw_position が -1 で pitch が 0 付近: 停止と判断し、最後の値を保持

### 検証結果（ユーザーからの報告）

**テスト日**: 2024-10-26

> 「raw_position は -1 になりますが、position は良い感じです。」

**動作確認**:
- ✅ **raw_position**: -1 になる（xwax の仕様通り、正常）
- ✅ **position**: 滑らかに動作する（pitch ベースの補間が機能）
- ✅ **逆回し時**: position が滑らかに減少
- ✅ **通常再生時**: position が滑らかに増加（引き続き正常）
- ✅ **停止時**: position が固定（Issue #1 で解決済み、引き続き正常）

### 修正ファイル

- ✅ `src/VinylTimecodeCHOP/VinylTimecodeCHOP.h`
  - `myLastNumSamples` メンバーを追加

- ✅ `src/VinylTimecodeCHOP/VinylTimecodeCHOP.cpp`
  - コンストラクタで `myLastNumSamples` を初期化
  - `processAudioSamples()` に pitch ベースの補間ロジックを追加

- ✅ `examples/Plugins/VinylTimecodeCHOP.plugin/` (再ビルド)

### テスト方法

1. **逆回しの速度変化テスト**:
   ```
   ゆっくり逆回し → 速く逆回し → ゆっくり
   → position が滑らかに変化することを確認 ✅
   ```

2. **通常再生との比較**:
   ```
   通常再生と逆回しで同じ滑らかさを持つことを確認 ✅
   ```

3. **Info CHOP での確認**:
   ```
   raw_position:  -1 が出る（正常）
   position:      滑らかに変化する ✅
   pitch:         負の値で安定
   ```

### バージョン

- **v1.1.0**: Issue #1 を解決（position = -1 問題）
- **v1.2.0**: Issue #2 を解決（逆回し時の飛び飛び問題）✅

---

## 📊 問題の履歴

### 解決済み

- **Issue #1**: position が -1 になる問題 (2024-10-26 解決) ✅
  - v1.1.0 で修正
  - 最後の有効な position を保持するロジックを実装

- **Issue #2**: 逆回し時の position が飛び飛びになる問題 (2024-10-26 解決) ✅
  - v1.2.0 で修正
  - pitch ベースの補間を実装

### 進行中

なし

---

**最終更新**: 2024-10-26
