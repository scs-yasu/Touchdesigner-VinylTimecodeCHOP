# Contributing to VinylTimecode CHOP

このプロジェクトへの貢献を歓迎します！Thank you for your interest in contributing to VinylTimecode CHOP!

## 🐛 バグ報告 / Bug Reports

バグを見つけた場合は、GitHubの[Issues](../../issues)で報告してください。

When reporting bugs, please include:

### 必要な情報 / Required Information

- **TouchDesignerバージョン** / TouchDesigner version
- **OS** (macOS version / Windows version)
- **オーディオインターフェース** / Audio interface (e.g., Native Instruments Interface 2)
- **使用バイナル** / Vinyl format (e.g., Pioneer RekordBox DVS Side A)
- **サンプルレート** / Sample rate (e.g., 44100 Hz)
- **再現手順** / Steps to reproduce
- **期待される動作** / Expected behavior
- **実際の動作** / Actual behavior
- **スクリーンショット** / Screenshots (if applicable)

### 例 / Example

```
環境 / Environment:
- OS: macOS 14.0
- TouchDesigner: 2023.11760
- Audio IF: Native Instruments Interface 2
- Vinyl: Pioneer RekordBox DVS Side A
- Sample Rate: 44100 Hz

問題 / Issue:
position は変化するが velocity が常に 0

再現手順 / Steps:
1. Audio Device In CHOP を 44100 Hz に設定
2. VinylTimecode CHOP を Pioneer RekordBox A に設定
3. レコードを回す
```

---

## 💡 機能リクエスト / Feature Requests

新機能のアイデアは大歓迎です！

New feature ideas are welcome! Please open an [Issue](../../issues) with:

- **機能の説明** / Feature description
- **使用例** / Use case
- **期待される動作** / Expected behavior
- **代替案** / Alternatives considered (if any)

---

## 🔧 プルリクエスト / Pull Requests

コード貢献は大歓迎です！以下の手順に従ってください。

Code contributions are welcome! Please follow these steps:

### 手順 / Steps

1. **フォーク** / Fork このリポジトリをフォーク
2. **ブランチ作成** / Create a branch
   ```bash
   git checkout -b feature/amazing-feature
   ```
3. **変更をコミット** / Commit your changes
   ```bash
   git commit -m 'Add amazing feature'
   ```
4. **プッシュ** / Push to your branch
   ```bash
   git push origin feature/amazing-feature
   ```
5. **プルリクエスト作成** / Open a Pull Request

### コミットメッセージ / Commit Messages

明確で簡潔なコミットメッセージを書いてください。

- 英語または日本語 OK
- 現在形を使用 ("Add feature" not "Added feature")
- 最初の行は50文字以内
- 詳細が必要な場合は空行の後に追加

### 例 / Examples

```bash
# Good
git commit -m "Add Traktor MK2 full support"
git commit -m "Fix velocity calculation for reverse playback"
git commit -m "Update README with new timecode format"

# Also good (日本語)
git commit -m "Traktor MK2 完全対応を追加"
git commit -m "逆再生時の velocity 計算を修正"
```

---

## 📝 コーディング規約 / Coding Standards

### C++ コード

- **スタイル**: C++11 以上
- **インデント**: 4スペース（タブではない）
- **命名規則**:
  - クラス: `PascalCase` (例: `TimecodeDecoder`)
  - 関数: `camelCase` (例: `getPosition`)
  - 変数: `snake_case` (例: `sample_rate`)
  - 定数: `UPPER_CASE` (例: `MAX_BUFFER_SIZE`)

### コメント

- 英語または日本語 OK
- 複雑なアルゴリズムには説明を追加
- パブリックAPIにはdocコメントを追加

```cpp
// Good
// Calculate position in samples using LFSR decoder
int position = timecoder_get_position(&tc);

// Also good (日本語)
// LFSR デコーダーを使用してサンプル位置を計算
int position = timecoder_get_position(&tc);
```

### ドキュメント

- README.md の更新が必要な場合は含める
- 新機能には使用例を追加
- 日本語と英語の両方で説明（可能な場合）

---

## 🧪 テスト / Testing

プルリクエストを送る前に：

Before submitting a pull request:

1. **ビルドが成功することを確認** / Ensure builds successfully
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

2. **動作確認** / Test functionality
   - TouchDesignerで実際に動作するか確認
   - 既存の機能が壊れていないか確認
   - 複数のタイムコードフォーマットで確認（可能な場合）

3. **ドキュメント更新** / Update documentation
   - README.md
   - CHANGELOG.md
   - 必要に応じてTROUBLESHOOTING.md

---

## 📄 ライセンス / License

このプロジェクトはGPL v3でライセンスされています。

This project is licensed under GPL v3.

貢献することで、あなたのコードもGPL v3の下でライセンスされることに同意したものとみなされます。

By contributing, you agree that your contributions will be licensed under GPL v3.

---

## 🤝 行動規範 / Code of Conduct

### 期待される行動 / Expected Behavior

- 敬意を持って接する / Be respectful
- 建設的なフィードバックを提供 / Provide constructive feedback
- 他者の視点を受け入れる / Accept differing viewpoints
- コミュニティを歓迎的に保つ / Keep the community welcoming

### 受け入れられない行動 / Unacceptable Behavior

- ハラスメント / Harassment
- 攻撃的な言動 / Offensive language
- スパム / Spam
- その他の非専門的な行為 / Other unprofessional conduct

---

## 💬 質問 / Questions

質問がある場合は、遠慮なく[Issues](../../issues)で質問してください。

If you have questions, feel free to ask in [Issues](../../issues).

---

## 🙏 謝辞 / Acknowledgments

貢献者の皆様、ありがとうございます！

Thank you to all contributors!

このプロジェクトは以下のプロジェクトに基づいています：
This project is based on:

- **xwax** by Mark Hills - タイムコーダー実装
- **Mixxx** - xwax統合とPioneer RekordBox定義

---

**Happy coding! 🎵🎬**
