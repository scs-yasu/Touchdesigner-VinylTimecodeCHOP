# GitHub公開前チェックリスト / Pre-Release Checklist

このチェックリストは、VinylTimecode CHOPをGitHubに公開する前に確認すべき項目です。

This checklist covers items to verify before publishing VinylTimecode CHOP to GitHub.

---

## 📋 コード / Code

### ソースコード / Source Code

- [ ] すべてのソースファイルが適切な著作権表示を含んでいる / All source files contain proper copyright notices
- [ ] GPL v3ライセンスヘッダーがソースファイルに含まれている / GPL v3 license headers are in source files
- [ ] デバッグコードやコメントアウトされたコードが削除されている / Debug code and commented-out code removed
- [ ] 個人情報や秘密情報が含まれていない / No personal or sensitive information included
- [ ] ハードコードされたパスが削除されている / No hardcoded paths

### ビルド / Build

- [ ] macOSでビルドが成功する / Builds successfully on macOS
- [ ] Windowsでビルドが成功する（テスト可能な場合）/ Builds successfully on Windows (if testable)
- [ ] CMakeLists.txtが正しく設定されている / CMakeLists.txt is properly configured
- [ ] ビルド成果物が.gitignoreで除外されている / Build artifacts are excluded in .gitignore

---

## 📄 ドキュメント / Documentation

### 必須ドキュメント / Required Documentation

- [ ] README.md が完成している / README.md is complete
  - [ ] 機能説明 / Feature description
  - [ ] インストール手順 / Installation instructions
  - [ ] 使用方法 / Usage instructions
  - [ ] トラブルシューティング / Troubleshooting
  - [ ] ライセンス情報 / License information
  - [ ] 謝辞 / Acknowledgments

- [ ] LICENSE ファイルが存在する / LICENSE file exists (GPL v3)

- [ ] CONTRIBUTING.md が存在する / CONTRIBUTING.md exists
  - [ ] バグ報告方法 / How to report bugs
  - [ ] 機能リクエスト方法 / How to request features
  - [ ] プルリクエストガイドライン / Pull request guidelines
  - [ ] コーディング規約 / Coding standards

- [ ] QUICKSTART.md が存在する / QUICKSTART.md exists

- [ ] TROUBLESHOOTING.md が存在する / TROUBLESHOOTING.md exists

- [ ] BUILD.md が存在する / BUILD.md exists

- [ ] CHANGELOG.md が最新 / CHANGELOG.md is up to date

### 追加ドキュメント / Additional Documentation

- [ ] docs/ ディレクトリが整理されている / docs/ directory is organized
  - [ ] video-sync-tutorial.md
  - [ ] success-config.md
  - [ ] v1.2.0-release-notes.md

### ドキュメントの品質 / Documentation Quality

- [ ] 日本語と英語の両方で記載されている（重要部分）/ Key sections in both Japanese and English
- [ ] リンク切れがない / No broken links
- [ ] スクリーンショットが含まれている（必要な場合）/ Screenshots included where needed
- [ ] サンプルコードが正しい / Sample code is correct

---

## 🗂️ ファイル構成 / File Structure

### 含めるファイル / Files to Include

- [ ] `src/` - ソースコード / Source code
- [ ] `lib/xwax/` - xwaxライブラリ / xwax library
- [ ] `examples/Plugins/` - サンプルプラグイン / Sample plugin
- [ ] `docs/` - ドキュメント / Documentation
- [ ] `CMakeLists.txt` - ビルド設定 / Build configuration
- [ ] `.gitignore` - Git除外設定 / Git ignore configuration
- [ ] `LICENSE` - ライセンス / License
- [ ] `README.md` - メインドキュメント / Main documentation
- [ ] `CONTRIBUTING.md` - コントリビューションガイド / Contribution guide
- [ ] `.github/ISSUE_TEMPLATE/` - Issueテンプレート / Issue templates

### 除外するファイル / Files to Exclude

- [ ] `build/` - ビルド成果物 / Build artifacts
- [ ] `*.toe`, `*.tox` - TouchDesigner作業ファイル / TouchDesigner project files
- [ ] `Backup/` - バックアップディレクトリ / Backup directory
- [ ] `mixxx-*/` - Mixxx参考実装 / Mixxx reference implementation
- [ ] `*.zip` - アーカイブファイル / Archive files
- [ ] `.claude/` - Claude AI設定 / Claude AI settings
- [ ] `.DS_Store` - macOSシステムファイル / macOS system files

---

## 🔧 Git設定 / Git Configuration

### .gitignore

- [ ] `.gitignore` が適切に設定されている / .gitignore is properly configured
- [ ] 個人設定が除外されている / Personal settings are excluded
- [ ] ビルド成果物が除外されている / Build artifacts are excluded
- [ ] 一時ファイルが除外されている / Temporary files are excluded

### GitHub設定 / GitHub Settings

- [ ] リポジトリ名を決定 / Repository name decided
  - 推奨: `VinylTimecodeCHOP`

- [ ] リポジトリの説明を用意 / Repository description prepared
  - 例: "TouchDesigner plugin for vinyl timecode control (DVS) - GPL v3"

- [ ] トピックタグを用意 / Topics prepared
  - 例: `touchdesigner`, `vinyl-control`, `dvs`, `timecode`, `audio-processing`

---

## 🧪 動作確認 / Testing

### プラグイン動作確認 / Plugin Testing

- [ ] macOSで動作確認済み / Tested on macOS
  - [ ] TouchDesignerバージョン: ___________
  - [ ] macOSバージョン: ___________

- [ ] Windowsで動作確認済み（可能な場合）/ Tested on Windows (if possible)
  - [ ] TouchDesignerバージョン: ___________
  - [ ] Windowsバージョン: ___________

### 機能確認 / Functionality Testing

- [ ] 複数のタイムコードフォーマットで動作確認 / Tested with multiple timecode formats
  - [ ] Serato CV02
  - [ ] Pioneer RekordBox DVS
  - [ ] Traktor Scratch

- [ ] 主要機能の動作確認 / Core functionality tested
  - [ ] position 検出 / Position detection
  - [ ] pitch 検出 / Pitch detection
  - [ ] 逆再生 / Reverse playback
  - [ ] 停止時の動作 / Stop behavior

---

## 📝 ライセンス / License

### GPL v3 コンプライアンス / GPL v3 Compliance

- [ ] LICENSEファイルが存在する / LICENSE file exists
- [ ] すべてのソースファイルに著作権表示がある / All source files have copyright notices
- [ ] xwaxの著作権表示が保持されている / xwax copyright notices are preserved
- [ ] Mixxxへの謝辞が含まれている / Acknowledgment to Mixxx is included

### 依存関係のライセンス / Dependency Licenses

- [ ] xwax (GPL v3) - 互換性OK / Compatible
- [ ] TouchDesigner SDK - 配布しない（ユーザーが各自取得）/ Not distributed
- [ ] ASIO SDK (Windows) - 配布しない（ユーザーが各自取得）/ Not distributed

---

## 🚀 GitHub公開 / GitHub Publishing

### リポジトリ作成 / Repository Creation

- [ ] GitHubアカウントを確認 / GitHub account verified
- [ ] リポジトリ作成準備完了 / Ready to create repository
- [ ] Public/Private選択（GPL v3推奨: Public）/ Visibility chosen (Recommended: Public)

### 初回コミット / Initial Commit

```bash
# チェックリスト確認後、以下を実行:

cd /Users/yasu/Desktop/Develop/touchdesignerOP

# Gitリポジトリ初期化
git init

# すべてのファイルを追加
git add .

# 初回コミット
git commit -m "Initial commit: VinylTimecode CHOP v1.2.0

- TouchDesigner plugin for vinyl timecode control
- Supports Serato, Pioneer RekordBox, Traktor formats
- Based on xwax timecoder implementation
- Tested with Native Instruments Interface 2
- GPL v3 licensed"

# GitHubリモートを追加（リポジトリURL作成後）
git remote add origin https://github.com/yourusername/VinylTimecodeCHOP.git

# メインブランチに変更
git branch -M main

# プッシュ
git push -u origin main
```

### リリース作成 / Create Release

- [ ] GitHubでv1.2.0タグを作成 / Create v1.2.0 tag on GitHub
- [ ] リリースノートを作成 / Create release notes
- [ ] ビルド済みプラグインを添付 / Attach compiled plugin
  - [ ] macOS版: VinylTimecodeCHOP.plugin
  - [ ] Windows版: VinylTimecodeCHOP.dll（可能な場合）

---

## 🎯 公開後の作業 / Post-Release Tasks

- [ ] README.mdのリポジトリURLを実際のURLに更新 / Update repository URLs in README.md
- [ ] GitHubバッジを追加 / Add GitHub badges
- [ ] コミュニティへのアナウンス（TouchDesignerフォーラムなど）/ Announce to community
- [ ] ドキュメントの更新（必要に応じて）/ Update documentation as needed

---

## ✅ 最終確認 / Final Check

すべてのチェックボックスにチェックが入りましたか？

Have all checkboxes been checked?

- [ ] **はい、すべて完了しました / Yes, everything is complete**

---

**公開準備完了！Good luck! 🎉**
