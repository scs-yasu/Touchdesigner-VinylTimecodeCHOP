# VinylTimecode CHOP for TouchDesigner

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Windows-lightgrey)](https://github.com/scs-yasu/Touchdesigner-VinylTimecodeCHOP)
[![TouchDesigner](https://img.shields.io/badge/TouchDesigner-2022.28000%2B-orange)](https://derivative.ca/)
[![GitHub](https://img.shields.io/badge/GitHub-VinylTimecodeCHOP-blue?logo=github)](https://github.com/scs-yasu/Touchdesigner-VinylTimecodeCHOP)

TouchDesigner用のバイナルタイムコード解析プラグイン。INTERFACE 2などのオーディオインターフェースから取得したタイムコードバイナルの信号を解析し、位置、速度、方向情報をリアルタイムで出力します。

## 🎉 動作確認済み！

**このプラグインは実際にテストされ、正常に動作することが確認されています。**

### 成功事例

```
✅ オーディオIF: Native Instruments Interface 2
✅ バイナル: Pioneer RekordBox DVS Side A
✅ サンプルレート: 44100 Hz
✅ スイッチ: LINE (PHONO ではない)
✅ TouchDesigner: 2023.11760
✅ macOS: Apple Silicon / Intel Universal Binary

動作時のパラメータ:
  position:        292652.53
  pitch:           1.0
  quality:         1.40
  valid_counter:   1639
  timecode_ticker: 7
```

**📖 すぐに始めたい方は [QUICKSTART.md](QUICKSTART.md) をご覧ください！**

## 特徴

- **複数フォーマット対応**
  - Serato CV02 (A/B/CD)
  - Traktor Scratch (標準)
  - Traktor MK2 (部分対応 - 制限あり)
  - Pioneer RekordBox DVS (A/B)
  - xwax 1kHz

- **低レイテンシ**
  - ASIO (Windows) / Core Audio (macOS) 直接アクセス
  - リアルタイム処理最適化

- **高精度**
  - Kalmanフィルタベースのピッチ推定
  - ノイズ耐性のあるゼロクロス検出
  - LFSR (Linear Feedback Shift Register) による位置特定

- **CHOP出力チャンネル**
  - `position`: レコード上の絶対位置 (0.0-1.0)
  - `velocity`: 回転速度のRPM倍率 (例: 1.0 = 33.3RPM)
  - `direction`: 回転方向 (1=前進, -1=後退)
  - `quality`: タイムコード信号品質 (0.0-1.0)

## 対応タイムコードフォーマット

| フォーマット | LFSR | 周波数 | 対応状況 | 備考 |
|------------|------|--------|---------|------|
| Serato CV02 A | 20-bit | 1000 Hz | ✅ 完全対応 | |
| Serato CV02 B | 20-bit | 1000 Hz | ✅ 完全対応 | |
| Serato CD | 20-bit | 1000 Hz | ✅ 完全対応 | |
| Traktor Scratch A | 23-bit | 2000 Hz | ✅ 完全対応 | MK1 vinyl |
| Traktor Scratch B | 23-bit | 2000 Hz | ✅ 完全対応 | MK1 vinyl |
| Traktor MK2 A | 110-bit | 2500 Hz | ⚠️ 部分対応 | Scratch A形式で代用可能 |
| Traktor MK2 B | 110-bit | 2500 Hz | ⚠️ 部分対応 | Scratch B形式で代用可能 |
| Pioneer RekordBox A | 20-bit | 1000 Hz | ✅ 完全対応 | rekordbox Control Vinyl |
| Pioneer RekordBox B | 20-bit | 1000 Hz | ✅ 完全対応 | rekordbox Control Vinyl |
| xwax 1kHz | 20-bit | 1000 Hz | ✅ 完全対応 | |

### 推奨バイナル

最も安定した動作が期待できるフォーマット:
1. **Serato CV02** - 最も広く使われている、安定性が高い
2. **Pioneer RekordBox DVS** - 最新の実装、Mixxxと互換性確認済み
3. **Traktor Scratch (MK1)** - 高解像度、Seratoより若干精度が高い

**避けるべき**: Traktor MK2 専用vinyl（完全対応困難、Scratch A/Bで代用推奨）

## 必要要件

### ハードウェア
- **バイナルタイムコードレコード**（上記対応フォーマット表を参照）
- ターンテーブル（33.3 RPM対応）
- **Native Instruments INTERFACE 2** または互換オーディオインターフェース
  - 44.1/48 kHz対応必須
  - ステレオライン入力
- TouchDesigner実行環境（macOS / Windows）

### ソフトウェア
- TouchDesigner 2022.28000以降
- CMake 3.15以降
- **Windows**: Visual Studio 2019以降
- **macOS**: Xcode 12以降

## ビルド手順

### 1. TouchDesigner SDKの取得

TouchDesignerインストールディレクトリから`CPlusPlus`サンプルをコピー:

```bash
# macOS
cp -r "/Applications/TouchDesigner.app/Contents/Resources/Samples/CPlusPlus/CHOP/" lib/touchdesigner-sdk/

# Windows
xcopy "C:\Program Files\Derivative\TouchDesigner\Samples\CPlusPlus\CHOP\" lib\touchdesigner-sdk\ /E /I
```

### 2. ASIO SDKの取得 (Windowsのみ)

Steinberg ASIO SDKをダウンロードして展開:

```bash
# lib/asiosdkに展開
wget https://www.steinberg.net/asiosdk -O asiosdk.zip
unzip asiosdk.zip -d lib/asiosdk
```

### 3. ビルド

```bash
# プロジェクトルートで
mkdir build
cd build

# macOS
cmake .. -G Xcode
open VinylTimecodeCHOP.xcodeproj

# Windows
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release

# Linux (experimental)
cmake .. -G "Unix Makefiles"
make
```

### 4. プラグインのインストール

ビルドが成功すると、プラグインは自動的に`examples/Plugins/`にコピーされます。

または、手動でTouchDesignerプラグインディレクトリにコピー:

```bash
# macOS
cp build/plugin/VinylTimecodeCHOP.plugin ~/Documents/Derivative/TouchDesigner/Plugins/

# Windows
copy build\plugin\VinylTimecodeCHOP.dll "%USERPROFILE%\Documents\Derivative\TouchDesigner\Plugins\"
```

## 使用方法

### 1. ハードウェアセットアップ

1. ターンテーブルにタイムコードバイナルをセット
2. INTERFACE 2をPCにUSB接続
3. ターンテーブルをINTERFACE 2のライン入力に接続
4. **INTERFACE 2のスイッチ設定**:
   - **LINE推奨**: フォノプリアンプ内蔵ターンテーブルまたは外部フォノプリアンプ使用時
   - **PHONO**: フォノプリアンプなしの場合（ただし増幅過多でクリッピングする可能性あり）
   - LEDが**緑色**になるよう調整（赤色はクリッピング）

### 2. TouchDesigner設定

1. TouchDesignerを起動
2. **Audio Device In CHOP**を作成:
   - Device: `INTERFACE2` を選択
   - Input Names: `0 1` (ステレオ入力)
   - Rate: `44100` Hz (**重要**: 44100 または 48000 を使用)
   - Buffer Length: `0.1` (100ms推奨)
   - Time Slice: `On`

3. **VinylTimecode CHOP**を作成:
   - Audio Device In CHOPを入力として接続
   - Timecode Format を選択
   - Sample Rate を Audio Device In と一致させる (44100 Hz推奨)

### 3. パラメータ設定

#### VinylTimecode CHOP

- **Timecode Format**: タイムコードフォーマットを選択
  - `Serato CV02 A/B/CD`
  - `Traktor Scratch A/B` (**MK2レコードでも動作可能**)
  - `Pioneer RekordBox A/B` (**rekordboxコントロールバイナル用**)
  - `xwax 1kHz`
  - `Auto Detect` (自動検出 - 非推奨)

- **Sample Rate**: **44100 Hz推奨** (Audio Device In CHOPと一致させる)
  - 48000 Hzも動作可能
  - **注意**: 60 Hzなど低いレートでは動作しません

#### Audio Device In CHOP

- **Device**: INTERFACE 2
- **Input Names**: `0 1` (ステレオ)
- **Rate**: `44100` Hz
- **Time Slice**: `On` (必須)

### 4. 出力の使用

出力チャンネルは他のCHOPと同様に使用できます:

```
# Mathチャンネルで速度をマッピング
position -> Lookup CHOP -> ビデオ位置制御
velocity -> Math CHOP -> エフェクトパラメータ
direction -> Logic CHOP -> スクラッチ検出
quality -> Feedback CHOP -> UI表示
```

## トラブルシューティング

### タイムコードが全く検出されない（position=0, velocity=0）

#### 1. サンプルレートを確認
- **Info CHOP** で `sample_rate` を確認
- **60 Hz** になっている場合は **問題あり**
- Audio Device In CHOPの **Rate を 44100 に設定**
- VinylTimecode CHOPの **Sample Rate を 44100 に設定**

#### 2. Audio Device In CHOPの設定確認
```
✓ Input Names: 0 1 (ステレオ)
✓ Rate: 44100
✓ Time Slice: On
✓ Number of Channels: 2
```

#### 3. タイムコードフォーマットが正しいか確認
- **Traktor MK2レコード** → `Traktor Scratch A/B` を試す
  - MK2完全対応は困難（110-bit LFSR）
  - Scratch A/B形式で部分的に動作可能
- **Pioneer rekordbox Control Vinyl** → `Pioneer RekordBox A/B`
- **Serato Control Vinyl** → `Serato CV02 A/B`

#### 4. 信号レベルを確認
- Audio Device In CHOPの波形を表示
- 振幅が ±0.02 〜 ±0.10 程度あればOK
- 波形が見えない場合:
  - Interface 2のスイッチ確認
  - ケーブル接続確認
  - ターンテーブルが再生中か確認

#### 5. Interface 2のLED確認
- **緑色**: OK
- **赤色**: クリッピング → LINEスイッチに変更

### タイムコード検出はされているが、velocity=0のまま

#### 現象
- `raw_position` に値がある
- `position` が変化している
- しかし `velocity` が 0 のまま
- `quality` が 0 または非常に低い

#### 原因と対策

1. **フォーマット不一致**
   - Traktor MK2レコードで Pioneer形式を選択している
   - 正しい形式に変更する

2. **Mixxx/rekordboxで動作確認**
   - 同じハードウェア構成でMixxxまたはrekordboxを起動
   - 正常に動作するか確認
   - 動作する場合: プラグイン側の問題
   - 動作しない場合: ハードウェア/ケーブルの問題

### レイテンシが高い

1. Audio Device In CHOPの Buffer Lengthを小さく (0.05〜0.1)
2. TouchDesignerの Perform モードで実行
3. 他のオーディオアプリケーションを終了
4. **注意**: バッファを小さくしすぎると音飛びが発生

### 信号品質（Quality）が低い

#### Quality値の目安
- **0.5以上**: 良好
- **0.2〜0.5**: 使用可能だが不安定
- **0.2未満**: 検出困難

#### 改善方法

1. **レコードのメンテナンス**
   - クリーニング（専用クリーナー使用）
   - 傷がないか確認
   - 正しいサイド（A/B）を使用

2. **針の状態**
   - 針圧を確認（メーカー推奨値）
   - 針の摩耗を確認
   - 針先のクリーニング

3. **電気的ノイズ対策**
   - ターンテーブルのアース接続確認
   - USB電源ノイズ（USBハブ経由の場合は直接接続）
   - ケーブルの配線（電源ケーブルから離す）

4. **Interface 2のゲイン**
   - LEDが緑色になる程度に調整
   - 赤色点灯 = クリッピング（NG）

### Traktor MK2レコードの制限事項

Traktor MK2コントロールバイナルは**110-bit LFSR + オフセット変調**を使用しており、完全な実装は非常に困難です。

#### 回避策
- **Traktor Scratch A/B** 形式を選択
- Quality は低くなりますが、position/velocity は取得可能
- より安定した動作が必要な場合:
  - Serato Control Vinylを使用
  - Pioneer RekordBox Control Vinylを使用

#### 技術的詳細
- MK2は23-bit + 110-bit dual LFSR方式
- 従来の振幅変調ではなくオフセット変調を使用
- 既存のxwaxライブラリでは対応不可

## 開発

### 重要な技術的注意事項

#### threshold値の扱い

xwaxライブラリでは、threshold値は **16ビット固定小数点数** として扱われます：

```cpp
// ❌ 間違い
threshold = 128

// ✅ 正しい
threshold = 128 << 16  // = 8,388,608
```

すべてのタイムコード定義で `threshold = 8388608` を使用してください。この値を間違えると、タイムコードが全く検出されません。

#### サンプルレート要件

- **最低**: 44100 Hz
- **推奨**: 44100 Hz または 48000 Hz
- **不可**: 60 Hz（TouchDesignerのデフォルトタイムスライスレート）

TouchDesignerでは、Audio Device In CHOPの **Time Slice を On** にして、適切なサンプルレートを確保してください。

#### オーディオ信号変換

float (-1.0〜1.0) から int16_t への変換:

```cpp
// Mixxxの実装と同様
int16_t sample = static_cast<int16_t>(float_sample * 32767.0f);
```

**注意**: 過度な増幅（amplification）はタイムコードパターンを破壊します。TouchDesigner側でゲイン調整を推奨します。

### プロジェクト構成

```
touchdesignerOP/
├── CMakeLists.txt              # ビルド設定
├── src/
│   ├── TimecodeDecoder/        # xwax timecoder移植
│   │   ├── timecoder.h         # メインヘッダ
│   │   ├── timecoder.cpp       # デコーダ実装
│   │   ├── lut.h               # ルックアップテーブル
│   │   ├── pitch.cpp           # Kalmanフィルタ
│   │   └── pitch.h
│   ├── AudioInput/             # オーディオ入力抽象化
│   │   ├── AudioInterface.h    # 抽象インターフェース
│   │   ├── ASIOInput.cpp       # Windows ASIO
│   │   ├── ASIOInput.h
│   │   ├── CoreAudioInput.cpp  # macOS Core Audio
│   │   └── CoreAudioInput.h
│   └── VinylTimecodeCHOP/      # TouchDesigner統合
│       ├── VinylTimecodeCHOP.h
│       └── VinylTimecodeCHOP.cpp
├── lib/
│   ├── asiosdk/                # ASIO SDK (Windows)
│   └── touchdesigner-sdk/      # TD Plugin SDK
├── examples/
│   ├── VinylTimecode.toe       # サンプルプロジェクト
│   └── Plugins/                # ビルド済みプラグイン
└── README.md
```

### 参考実装

このプロジェクトは以下の実装を参考にしています:

- **[Mixxx DJ Software](https://github.com/mixxxdj/mixxx)** - オープンソースDJソフトウェア
  - xwax timecoder の実装を参考
  - Pioneer RekordBox 定義の取得元
  - `lib/xwax/timecoder.c` に全タイムコード定義あり
  - `src/vinylcontrol/vinylcontrolxwax.cpp` に統合実装

- **[xwax](https://www.xwax.co.uk/)** - Mark Hills氏によるDigital Vinyl System
  - オリジナルのタイムコーダー実装
  - LFSR (Linear Feedback Shift Register) アルゴリズム
  - Kalmanフィルタベースのピッチ推定

#### Mixxxから学んだ重要なポイント

1. **threshold = (128 << 16)** の固定小数点表現
2. **phono パラメータは false** で初期化
3. サンプル変換時の **SAMPLE_MAXIMUM (32767)** の使用
4. Pioneer RekordBox の正確な LFSR パラメータ
5. タイムコーダーへのサンプル送信は **ステレオフレーム数** で指定

## ライセンス

このプロジェクトはGNU General Public License v3.0の下でライセンスされています。

このプロジェクトは、Mark Hillsによる[xwax](https://www.xwax.co.uk/)のタイムコーダー実装を含んでおり、これもGPL v3でライセンスされています。

詳細については、[LICENSE](LICENSE)ファイルを参照してください。

### 重要なライセンス要件

GPL v3に基づき、このソフトウェアを使用、変更、配布する際は：

1. **ソースコードの公開**: このプラグインを配布する場合、ソースコードも提供する必要があります
2. **同一ライセンス**: 派生物もGPL v3でライセンスする必要があります
3. **著作権表示**: オリジナルの著作権表示を保持する必要があります
4. **変更の記録**: 変更を加えた場合、その旨を明記する必要があります

商用利用も可能ですが、上記の要件を満たす必要があります。

## 謝辞

このプロジェクトは以下の優れたオープンソースプロジェクトに基づいています：

- **xwax** - Mark Hills氏によるデジタルバイナルシステム
  - タイムコーダー実装の元となったコード
  - ライセンス: GPL v3
  - https://www.xwax.co.uk/

- **Mixxx DJ Software** - オープンソースDJソフトウェア
  - xwax timecoderの統合実装を参考
  - ライセンス: GPL v2+
  - https://github.com/mixxxdj/mixxx

- **TouchDesigner** - Derivative Inc.
  - Plugin SDK
  - https://derivative.ca/

## 著作権

Copyright (C) 2025 VinylTimecode Team
Copyright (C) 2021 Mark Hills (xwax timecoder implementation)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

## 🤝 コントリビューション / Contributing

バグ報告、機能リクエスト、プルリクエストを歓迎します！

We welcome bug reports, feature requests, and pull requests!

詳細は[CONTRIBUTING.md](CONTRIBUTING.md)をご覧ください。

For details, please see [CONTRIBUTING.md](CONTRIBUTING.md).

### 貢献方法 / How to Contribute

1. **バグ報告** / Bug Reports
   - [Issue](../../issues)を作成してバグを報告
   - テンプレートに従って環境情報を記入

2. **機能リクエスト** / Feature Requests
   - [Issue](../../issues)で新機能を提案
   - 使用例と期待される動作を記入

3. **プルリクエスト** / Pull Requests
   - フォークしてブランチを作成
   - 変更をコミットしてプルリクエスト
   - コーディング規約に従う

---

## サポート / Support

問題や質問がある場合は、GitHubの[Issues](../../issues)で報告してください。

For issues or questions, please report them on GitHub [Issues](../../issues).
