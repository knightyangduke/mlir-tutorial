# Git Cheatsheet

## 查看信息

```bash
# 查看当前状态
git status

# 查看所有分支（本地 + 远程）
git branch -a

# 仅本地分支
git branch

# 仅远程分支
git branch -r

# 查看分支跟踪关系
git branch -vv

# 查看远程仓库
git remote -v

# 查看提交历史
git log --oneline --graph --all
```

## 创建远程分支

远程分支本质上是把本地分支推送到远程，没有单独的 `git remote branch` 命令。

```bash
# 方法一：全新分支（推荐）
git checkout -b my-new-branch    # 从当前 HEAD 创建新分支（本地）
# ... 做一些修改、提交 ...
git push -u origin my-new-branch # 推送到远程 → 远程才出现 origin/my-new-branch

# 方法二：基于某个远程分支创建
git checkout -b my-new-branch origin/main  # 基于 origin/main 创建（本地）
git push -u origin my-new-branch           # 推送到远程

# 方法三：已有本地分支，直接推送
git push -u origin feature-x
```

> `git push -u origin <branch>` 中的 `-u`（`--set-upstream`）同时做两件事：
> 1. 在远程创建同名分支
> 2. 设置本地分支跟踪该远程分支，之后 `git push` / `git pull` 无需指定参数

## 分支操作

> **关键区别**：`git branch` 和 `git checkout -b` **都只创建本地分支**，不会触碰远程。
> 只有 `git push -u origin <branch>` 才会在远程创建分支。

```bash
# 创建新分支（仅本地，不切换）
git branch <branch-name>

# 创建并切换到新分支（仅本地）
git checkout -b <branch-name>
git switch -c <branch-name>

# 切换分支
git checkout <branch-name>
# 或
git switch <branch-name>

# 删除本地分支
git branch -d <branch-name>

# 删除远程分支
git push origin --delete <branch-name>
```

## 远程分支跟踪

```bash
# 拉取远程分支到本地并自动跟踪
git checkout <branch-name>          # 如果远程有同名分支，自动跟踪
git switch <branch-name>

# 创建本地分支并跟踪指定远程分支
git checkout -b <local-branch> origin/<remote-branch>

# 为已有本地分支设置/修改跟踪的远程分支
git branch -u origin/<branch-name>
# 或指定分支名
git branch --set-upstream-to=origin/<branch-name> <local-branch>

# 推送本地分支到远程并设置跟踪（首次推送时）
git push -u origin <branch-name>
```

### 示例

假设你有以下远程分支：

```
origin/HEAD -> origin/main
origin/main
origin/new_poly_pattern
```

| 目标 | 命令 |
|------|------|
| 切换到 `main`（已跟踪 `origin/main`） | `git checkout main` |
| 切换到 `new_poly_pattern`（已跟踪 `origin/new_poly_pattern`） | `git checkout new_poly_pattern` |
| 从当前分支切换到 `main` 并保持跟踪 | `git checkout main` |
| 让当前分支改跟踪 `origin/main` | `git branch -u origin/main` |

## 推送与拉取

```bash
# 推送到跟踪的远程分支
git push

# 推送到指定远程和目标分支
git push origin <local-branch>:<remote-branch>

# 拉取远程更新
git pull

# 拉取但不自动合并
git fetch
```

> **注意**：`git push` 默认推送到当前分支所跟踪的远程分支，**不是** `origin/main`。
>
> 例如在 `new_poly_pattern` 上执行 `git push`，会推到 `origin/new_poly_pattern`。

## 常用工作流

```bash
# 1. 更新 main
git checkout main
git pull

# 2. 创建功能分支
git checkout -b feature/my-feature

# 3. 开发后推送
git push -u origin feature/my-feature

# 4. 后续推送
git push
```

## 撤销操作

```bash
# 撤销工作区修改（未暂存）
git restore <file>

# 取消暂存
git restore --staged <file>

# 修改最近一次 commit
git commit --amend

# 重置到某个提交（谨慎使用）
git reset --hard <commit-hash>
```
