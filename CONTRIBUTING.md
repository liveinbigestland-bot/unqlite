# 贡献指南

感谢你考虑为 UnQLite 做贡献！请遵循以下指南。

## 快速开始

1. **Fork** 本仓库
2. **Clone** 到本地
3. 从 `develop` 创建功能分支：`git checkout -b feature/your-feature develop`
4. 开发、测试、提交
5. **Push** 到你的 fork
6. 创建 **Pull Request** 到 `develop` 分支

## 分支策略

| 分支 | 用途 | 保护 |
|------|------|------|
| `master` | 稳定发布版本 | 🔒 受保护，仅通过 PR 合并 |
| `develop` | 开发主线 | 🔒 受保护，仅通过 PR 合并 |
| `feature/*` | 新功能开发 | 合并后删除 |
| `fix/*` | Bug 修复 | 合并后删除 |
| `hotfix/*` | 紧急修复 | 从 master 分出，合并回 master + develop |

## Commit 规范

使用 [Conventional Commits](https://www.conventionalcommits.org/) 格式：

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

**Type**:
- `feat`: 新功能
- `fix`: Bug 修复
- `docs`: 文档变更
- `refactor`: 重构（不改变功能）
- `perf`: 性能优化
- `test`: 测试相关
- `build`: 构建系统或依赖
- `ci`: CI/CD 配置
- `chore`: 其他杂项

**示例**:
```
feat(kv): add safe allocation macro to prevent integer overflow
fix(vfs): resolve 64-bit truncation in file read operation
test(pager): add ACID transaction tests
```

## 代码规范

### C 语言
- 遵循现有代码风格（参考项目中已有的命名和缩进）
- 缩进使用 **Tab**
- 所有内存分配必须检查 NULL 返回值
- 使用 `SyMemBackendAlloc` 而非 `malloc`
- 使用 `Systrcpy` 而非 `strcpy`/`sprintf`
- 乘法分配前检查整数溢出
- goto 仅用于错误清理路径，标签使用 `err_xxx` / `cleanup`

### 错误处理
- 函数返回 `sxi32` / `int` 错误码
- 使用 `SXRET_OK` 表示成功
- 错误路径必须释放已分配的资源

### 审查重点
1. **内存安全**：NULL 检查、溢出保护、Use-After-Free
2. **整数溢出**：`n * sizeof()` 模式
3. **资源泄漏**：goto 清理路径完整性
4. **线程安全**：互斥锁的正确使用

## Pull Request 要求

- [ ] 通过所有 CI 检查（编译 + 测试 + 静态分析）
- [ ] 新增代码有对应的测试
- [ ] 不降低测试覆盖率
- [ ] 至少 1 人 Code Review 通过
- [ ] Commit 消息符合规范
- [ ] 安全相关修改需要架构师 (Lead) 审核

## Issue 提交

提交 Bug 时请包含：
- 复现步骤
- 期望行为 vs 实际行为
- UnQLite 版本和平台信息
- 最小复现代码（如可能）
