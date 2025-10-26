# VinylTimecodeCHOP - ビデオ同期チュートリアル

このチュートリアルでは、VinylTimecode CHOP の position を使って動画を制御する方法を説明します。

---

## 🎯 実現すること

- ✅ レコードを回すと動画が再生される
- ✅ レコードを止めると動画も止まる
- ✅ レコードを逆回しすると動画も逆再生される
- ✅ レコードの速度に応じて動画の速度が変わる

---

## 📋 必要なもの

1. **VinylTimecodeCHOP プラグイン** (v1.2.0 以降)
2. **動画ファイル** (.mp4, .mov など)
3. **Native Instruments Interface 2** (または互換オーディオIF)
4. **Pioneer RekordBox DVS バイナル** (または対応バイナル)

---

## 🔧 TouchDesigner ネットワーク構成

### 基本構成

```
[Audio Device In CHOP] → [VinylTimecode CHOP] → [Math CHOP] → [Movie File In TOP]
                                                                         ↓
                                                                  [Null TOP (出力)]
```

---

## ステップ 1: Audio Device In CHOP の設定

1. **CHOP を作成**: `Audio Device In CHOP` を作成

2. **パラメータ設定**:
   ```
   Driver:       Core Audio
   Device:       Native Instruments Interface 2
   Rate:         44100
   Time Slice:   On
   Input Names:  0 1
   Num Channels: 2
   ```

3. **名前を変更**: `audioDevin1` など分かりやすい名前に

---

## ステップ 2: VinylTimecode CHOP の設定

1. **CHOP を作成**: Tab キーを押して `Vinyltimecode` と入力

2. **接続**: `Audio Device In CHOP` の出力を接続

3. **パラメータ設定**:
   ```
   Format: Pioneer RekordBox Side A (または使用するバイナル)
   ```

4. **名前を変更**: `vinyltimecode1` など

---

## ステップ 3: Math CHOP で position を動画用に変換

### 3-1. position を秒単位に変換

1. **Math CHOP を作成**: `Math CHOP` を作成

2. **接続**: `VinylTimecode CHOP` を接続

3. **OP タブの設定** (デフォルトのまま OK):
   ```
   Combine CHOPs:    Off
   Channel Pre Op:   Off
   Align:            Extend to Min/Max
   ```

4. **Channel Names**: `position` のみを使用
   - 他のチャンネルを削除したい場合は `Select CHOP` を使用

5. **Multi-Add タブ** で計算方法を設定:
   ```
   Operation: Multiply
   Multiply: 0.0000226757
   ```

   **計算式の説明**:
   ```
   0.0000226757 = 1 ÷ 44100

   position (サンプル) × (1 / 44100) = position (秒)
   例: 44100 サンプル × 0.0000226757 = 1.0 秒
   ```

6. **名前を変更**: `math_position_to_seconds`

**重要**: 設定する場所は **Multi-Add タブ** の Multiply パラメータです！

**Math CHOP のタブ構成**:
```
┌─────────────────────────────────────┐
│ OP | Multi-Add | Common             │ ← タブ
├─────────────────────────────────────┤
│                                     │
│ Multi-Add タブを開く ← ここで設定！ │
│                                     │
│ Operation:  [Multiply ▼]            │
│ Multiply:   [0.0000226757]          │
│ Add:        [0]                     │
│ ...                                 │
└─────────────────────────────────────┘
```

**注意**:
- ❌ **OP タブ** の Channel Pre Op ではありません
- ✅ **Multi-Add タブ** の Operation と Multiply です

### 3-2. 秒をフレーム番号に変換

1. **Math CHOP を作成**: もう1つ `Math CHOP` を作成

2. **接続**: `math_position_to_seconds` を接続

3. **Multi-Add タブ** で計算方法を設定:
   ```
   Operation: Multiply
   Multiply: 30
   ```

   **動画のフレームレートに合わせる**:
   - 30 FPS の動画: `Multiply: 30`
   - 60 FPS の動画: `Multiply: 60`
   - 24 FPS の動画: `Multiply: 24`

   **計算式の説明**:
   ```
   position (秒) × FPS = frame number

   例: 2.5 秒 × 30 FPS = 75 フレーム目
   ```

4. **名前を変更**: `math_position_to_frame`

**重要**: ここも **Multi-Add タブ** の Multiply パラメータです！

---

## ステップ 4: Movie File In TOP の設定

1. **Movie File In TOP を作成**: `Movie File In TOP` を作成

2. **動画ファイルを読み込み**:
   ```
   File: /path/to/your/video.mp4
   ```

3. **パラメータ設定**:
   ```
   Play Mode:           Specify Index
   Index:               math_position_to_frame[0, 0]
                        (CHOP リファレンスを使用)
   
   Extend:              Hold
   または
   Extend:              Loop  (ループ再生したい場合)
   
   Post-Play Behavior:  Hold Last Frame
   または
   Post-Play Behavior:  Loop  (ループ再生したい場合)
   
   Independent Time:    On (重要!)
   ```

### CHOP リファレンスの設定方法

1. **Index パラメータを右クリック** → `CHOP Reference...` を選択

2. **CHOP Reference ダイアログ**:
   ```
   CHOP:    math_position_to_frame
   Channel: position (または position のチャンネル名)
   Sample:  0
   ```

3. **または直接入力**:
   ```
   Index: op('math_position_to_frame')['position', 0]
   ```

4. **名前を変更**: `moviefilein_vinyl`

---

## ステップ 5: 出力用 Null TOP

1. **Null TOP を作成**: `moviefilein_vinyl` に接続

2. **名前を変更**: `OUT_vinyl_video`

3. **ビューワーで確認**: Null TOP をアクティブにして動画を確認

---

## 🎮 完全なネットワーク構成

```
┌─────────────────────┐
│ Audio Device In     │
│ (Interface 2)       │
│ - Rate: 44100       │
│ - Time Slice: On    │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ VinylTimecode CHOP  │
│ - Format: Pioneer A │
│                     │
│ Outputs:            │
│ - position          │
│ - pitch             │
│ - quality           │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Math CHOP (秒変換)  │
│ Multiply: 1/44100   │
│                     │
│ position (samples)  │
│ → position (seconds)│
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Math CHOP (フレーム)│
│ Multiply: 30 (FPS)  │
│                     │
│ position (seconds)  │
│ → frame number      │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Movie File In TOP   │
│ Play Mode: Index    │
│ Index: ↑ CHOP Ref   │
│ Independent: On     │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Null TOP (OUT)      │
│                     │
│ 最終出力            │
└─────────────────────┘
```

---

## ⚙️ 詳細設定と調整

### 動画の速度を調整したい場合

**Math CHOP (フレーム変換) で調整**:

```
Multiply: 30  (通常速度)
Multiply: 60  (2倍速)
Multiply: 15  (半速)
```

### 動画の開始位置を調整したい場合

**Math CHOP (秒変換) の後に Add オペレーション**:

1. **Math CHOP を追加**:
   ```
   Operation: Add
   Add: 10  (10秒から開始)
   ```

### ループ再生したい場合

**Math CHOP で Modulo オペレーション**:

1. **動画の長さを確認** (例: 60秒、30 FPS = 1800フレーム)

2. **Math CHOP を追加** (`math_position_to_frame` の後):
   ```
   Operation: Modulo
   Modulo: 1800  (動画の総フレーム数)
   ```

3. **Movie File In TOP**:
   ```
   Extend: Hold (Modulo で制御するため)
   ```

---

## 🎯 応用例

### 例 1: スクラッチエフェクト

**Math CHOP で pitch を可視化**:

```
[VinylTimecode CHOP] → [Select CHOP: pitch] → [Trail CHOP] → [Feedback效果]
```

### 例 2: 複数の動画を切り替え

**Switch TOP で動画を切り替え**:

```
[Movie File In TOP 1] ─┐
[Movie File In TOP 2] ─┤→ [Switch TOP] → [Null TOP]
[Movie File In TOP 3] ─┘
                        ↑
            [Math CHOP: position / 10000 で切り替え]
```

### 例 3: エフェクト追加

**position に応じてエフェクトを変化**:

```
[Movie File In TOP]
        ↓
[Blur TOP: blur amount = pitch の絶対値]
        ↓
[Level TOP: brightness = quality]
        ↓
[Null TOP]
```

---

## 🐛 トラブルシューティング

### 問題 1: 動画が動かない

**チェック項目**:
1. ✅ Movie File In の `Play Mode` が `Specify Index` か？
2. ✅ `Independent Time` が `On` か？
3. ✅ CHOP リファレンスが正しく設定されているか？
4. ✅ VinylTimecode CHOP の position が変化しているか？（Info CHOP で確認）

**確認方法**:
```python
# Textport で確認
n = op('math_position_to_frame')
print(n['position', 0])  # フレーム番号が出力されるか確認
```

### 問題 2: 動画がガクガクする

**解決策**:
1. **Audio Device In CHOP**:
   ```
   Time Slice: On (必須)
   ```

2. **VinylTimecode CHOP**:
   ```
   Common タブ:
   Cook Every Frame: Off
   ```

3. **Movie File In TOP**:
   ```
   Preload Mode: On Demand
   または
   Preload Mode: Preload Some
   ```

### 問題 3: 逆再生時に動画が停止する

**原因**:
position が負の値になっている

**解決策**:
position を絶対値に変換する Math CHOP を追加:

```
Operation: Abs (絶対値)
```

**または**、開始位置を動画の中間に設定:

```
Math CHOP (Add):
Add: 900  (動画の中間フレーム)
```

### 問題 4: 動画とレコードの速度が合わない

**調整方法**:
Math CHOP (フレーム変換) の `Multiply` 値を調整:

```
速すぎる場合: Multiply を減らす (例: 30 → 15)
遅すぎる場合: Multiply を増やす (例: 30 → 60)
```

---

## 💡 ヒントとコツ

### 1. position のスケーリング

動画の長さに合わせて position をスケーリング:

```
動画の長さ: 60秒 = 2646000 サンプル (44100 Hz)
VinylTimecode の position: 0 ~ 2646000 で動画全体を再生
```

### 2. 複数のバイナルを使用

複数のターンテーブルで複数の動画を制御:

```
[Audio Device In 1] → [VinylTimecode 1] → [Movie 1]
[Audio Device In 2] → [VinylTimecode 2] → [Movie 2]
```

### 3. パフォーマンス最適化

**大きな動画ファイルの場合**:

1. **動画を軽量化**:
   ```
   解像度: 1920x1080 → 1280x720
   コーデック: H.264
   ビットレート: 適度に圧縮
   ```

2. **SSD に配置**: 読み込み速度が重要

3. **Preload 設定**:
   ```
   Preload Mode: Preload Some
   Preload Frames: 60
   ```

### 4. Info CHOP でデバッグ

VinylTimecode CHOP の Info CHOP を常に表示:

```
Info CHOP チャンネル:
- position:     現在のサンプル位置
- raw_position: 生の値（-1 チェック用）
- pitch:        速度
- quality:      信号品質
```

---

## 📊 設定例のまとめ

### シンプルな構成（推奨）

```
┌──────────────────┐
│ audioDevin1      │ Rate: 44100, Time Slice: On
└────────┬─────────┘
         │
┌────────▼─────────┐
│ vinyltimecode1   │ Format: Pioneer RekordBox Side A
└────────┬─────────┘
         │
┌────────▼─────────┐
│ math1            │ Multiply: 1/44100 (秒に変換)
└────────┬─────────┘
         │
┌────────▼─────────┐
│ math2            │ Multiply: 30 (30 FPS の場合)
└────────┬─────────┘
         │
┌────────▼─────────┐
│ moviefilein1     │ Play Mode: Specify Index
│                  │ Index: op('math2')['position', 0]
│                  │ Independent Time: On
└────────┬─────────┘
         │
┌────────▼─────────┐
│ out1             │ 最終出力
└──────────────────┘
```

### パラメータ一覧

| Operator | タブ | Parameter | Value |
|----------|------|-----------|-------|
| **audioDevin1** | Device | Driver | Core Audio |
| | | Device | Interface 2 |
| | | Rate | 44100 |
| | Time Slice | Time Slice | On |
| | | Input Names | 0 1 |
| **vinyltimecode1** | Parameters | Format | Pioneer RekordBox Side A |
| **math1** | **Multi-Add タブ** | **Operation** | **Multiply** |
| | | **Multiply** | **0.0000226757** (1/44100) |
| **math2** | **Multi-Add タブ** | **Operation** | **Multiply** |
| | | **Multiply** | **30** (動画の FPS) |
| **moviefilein1** | Play | Play Mode | Specify Index |
| | | Index | `op('math2')['position', 0]` |
| | | Independent Time | On |
| | | Extend | Hold または Loop |

---

## 📌 クイックリファレンス

### Math CHOP の設定場所

**Math CHOP #1 (秒に変換)**:
```
1. Math CHOP を作成
2. Multi-Add タブを開く ← 重要！
3. Operation: Multiply を選択
4. Multiply: 0.0000226757 と入力
```

**Math CHOP #2 (フレームに変換)**:
```
1. Math CHOP を作成
2. Multi-Add タブを開く ← 重要！
3. Operation: Multiply を選択
4. Multiply: 30 と入力 (30 FPS の場合)
```

### タブの場所

```
Math CHOP のパラメータウィンドウ:
┌──────────────────────────────────┐
│ OP | Multi-Add | Common          │ ← タブバー
└──────────────────────────────────┘
     ^^^^^^^^^
     ここをクリック！

Multi-Add タブの中身:
┌──────────────────────────────────┐
│ Operation:  [Multiply ▼]         │ ← ここで Multiply を選択
│ Multiply:   [0.0000226757]       │ ← ここに数値を入力
│ Add:        [0]                  │
│ Subtract:   [0]                  │
│ ...                              │
└──────────────────────────────────┘
```

---

## 🎉 完成！

これで、バイナルレコードで動画を完全にコントロールできます：

- ✅ **再生**: レコードを回す → 動画が再生される
- ✅ **停止**: レコードを止める → 動画が止まる
- ✅ **逆再生**: レコードを逆回し → 動画が逆再生される
- ✅ **速度変化**: レコードの速度 → 動画の速度が変わる

---

## 📚 関連ドキュメント

- [QUICKSTART.md](../QUICKSTART.md) - VinylTimecodeCHOP のセットアップ
- [TROUBLESHOOTING.md](../TROUBLESHOOTING.md) - トラブルシューティング
- [docs/success-config.md](success-config.md) - 成功構成の詳細
- [docs/v1.2.0-release-notes.md](v1.2.0-release-notes.md) - 最新バージョン情報

---

**Happy VJ-ing with Vinyl!** 🎵🎬
