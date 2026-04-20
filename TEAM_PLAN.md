# UnQLite 团队组织方案 (TEAM_PLAN)

> **基于代码审计报告的 3 人团队规划**  
> **目标**: 系统性提升代码质量、可维护性和测试覆盖率

---

## 一、团队角色分配

### 1.1 角色定义

| 角色 | 代号 | 核心职责 | 关注领域 |
|------|------|---------|---------|
| **架构师 / 技术负责人 (Lead)** | A | 架构决策、代码审查、技术指导 | API 设计、内存安全、构建系统 |
| **核心开发者 (Developer)** | B | Bug 修复、功能开发、重构 | Jx9 VM/编译器、KV 引擎 |
| **质量工程师 (QA Engineer)** | C | 测试框架、自动化测试、持续集成 | 测试覆盖、静态分析、CI/CD |

### 1.2 技能要求

**角色 A - 架构师**:
- 精通 C 语言和嵌入式系统开发
- 熟悉数据库内部原理（B+树、哈希、事务、WAL）
- 有大代码库维护和重构经验
- 能力要求：5 年以上 C 开发经验

**角色 B - 核心开发者**:
- 扎实的 C 语言基础，理解内存管理和指针操作
- 熟悉编译原理（字节码、虚拟机）优先
- 有系统编程（OS/VFS/文件系统）经验优先
- 能力要求：2 年以上 C 开发经验

**角色 C - 质量工程师**:
- 熟悉 C 语言测试框架（Unity、Ceedling、Check 等）
- 了解静态分析工具（Coverity、cppcheck、clang-tidy）
- 有 CI/CD 搭建经验（GitHub Actions、GitLab CI）
- 能力要求：2 年以上测试开发经验

---

## 二、分阶段工作计划

### Phase 0: 基础设施搭建（第 1-2 周）

**负责人**: 角色 C（主导）+ 角色 A（协助）

| 任务 | 优先级 | 预估工时 | 详情 |
|------|--------|---------|------|
| 改进 CMakeLists.txt | 🔴 P0 | 2d | 添加编译警告、调试/发布模式、Sanitizer 选项 |
| 搭建测试框架 | 🔴 P0 | 3d | 选择并集成测试框架（推荐 Unity/Ceedling） |
| 建立基础测试用例 | 🔴 P0 | 3d | KV 存储基本 CRUD 测试、Jx9 基本执行测试 |
| 配置 CI/CD | 🟡 P1 | 2d | GitHub Actions 自动构建 + 测试 |
| 添加静态分析 | 🟡 P1 | 2d | 集成 cppcheck + clang-tidy |

**CMakeLists.txt 改进清单**:
```cmake
# 添加编译器警告
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Werror -Wpedantic)
    add_compile_options(-fno-omit-frame-pointer)
endif()

# 添加 Sanitizer 选项
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)

# 添加调试/发布模式优化
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
```

**交付物**:
- [ ] 改进后的 CMakeLists.txt
- [ ] 测试框架集成
- [ ] 至少 10 个基础测试用例
- [ ] CI 配置文件
- [ ] 静态分析配置

---

### Phase 1: 紧急缺陷修复（第 3-4 周）

**负责人**: 角色 B（主导）+ 角色 A（审查）

| 任务 | 优先级 | 预估工时 | 详情 |
|------|--------|---------|------|
| 修复 64-bit 截断 | 🔴 P0 | 1d | `jx9_vfs.c:4290` FIXME |
| 内存分配 NULL 检查 | 🔴 P0 | 3d | 审查并修复全部 44 处分配 |
| 整数溢出保护 | 🔴 P0 | 2d | 添加安全分配宏 |
| 路径缓冲区安全 | 🟡 P1 | 1d | 替换固定 4096 栈缓冲区 |
| TODO/HACK 清理 | 🟡 P1 | 2d | 处理 4 处待处理标记 |

**安全分配宏实现**（角色 B 交付）:
```c
// 在 unqliteInt.h 中添加
#define UNQLITE_SAFE_MUL(a, b, limit) \
    ((b) == 0 ? 0 : ((a) > (limit) / (b) ? (limit) : (a) * (b)))

#define SyMemBackendSafeAlloc(backend, count, type) \
    ((count) > SX_MAX_ALLOC / sizeof(type) ? NULL : \
     (type *)SyMemBackendAlloc((backend), (count) * sizeof(type)))
```

**交付物**:
- [ ] 64-bit 截断修复的补丁
- [ ] 44 处内存分配的 NULL 检查
- [ ] 安全分配宏 + 全局替换
- [ ] 对应的单元测试

---

### Phase 2: 代码质量提升（第 5-8 周）

**负责人**: 角色 A（主导）+ 角色 B（开发）

#### 2A: 大文件拆分（角色 A + B）

| 文件 | 当前行数 | 拆分方案 | 预估工时 |
|------|---------|---------|---------|
| `jx9_builtin.c` | 8297 | 按功能域拆分：字符串函数、数组函数、文件IO函数、数学函数、JSON函数 | 5d |
| `jx9_vm.c` | 7135 | 拆分为：VM 核心、指令分发、栈操作、函数调用 | 5d |
| `jx9_compile.c` | 3689 | 拆分为：词法分析辅助、语法分析、代码生成、优化 | 3d |

**拆分原则**:
1. 保持 `jx9_builtin.c` 对外 API 不变（内部调用改为不同 .c 文件）
2. 每个拆分后的文件不超过 2000 行
3. 头文件保持公共接口不变
4. 所有拆分通过现有测试验证

#### 2B: goto 标签规范化（角色 B）

| 步骤 | 任务 | 预估工时 |
|------|------|---------|
| 1 | 制定 goto 命名规范文档 | 0.5d |
| 2 | 规范化 `jx9_lib.c`（92处） | 2d |
| 3 | 规范化 `jx9_vfs.c`（90处） | 2d |
| 4 | 规范化 `jx9_vm.c`（83处） | 2d |
| 5 | 规范化其余文件 | 2d |

**Goto 命名规范**:
```
cleanup    - 通用清理
err_alloc  - 内存分配失败
err_io     - IO 操作失败
err_lock   - 锁操作失败
err_format - 数据格式错误
err_param  - 参数错误
```

#### 2C: 错误处理改进（角色 A）

| 任务 | 预估工时 |
|------|---------|
| 设计扩展错误码体系 | 1d |
| 实现 `unqlite_strerror()` | 1d |
| 统一 API NULL 指针检查 | 2d |
| 完善错误处理文档 | 1d |

**扩展错误码建议**:
```c
// 新增语义化错误码
#define UNQLITE_EINVAL     -2   // 无效参数
#define UNQLITE_EPERM      -3   // 权限不足
#define UNQLITE_ENOSPC     -4   // 空间不足
#define UNQLITE_EFORMAT    -5   // 数据格式错误
#define UNQLITE_EINTERNAL  -6   // 内部错误
#define UNQLITE_ETOOLARGE  -7   // 数据过大
```

---

### Phase 3: 测试体系完善（第 5-10 周，与 Phase 2 并行）

**负责人**: 角色 C（主导）

| 阶段 | 任务 | 测试覆盖率目标 | 预估工时 |
|------|------|--------------|---------|
| 3A | KV 存储引擎测试 | 60% | 5d |
| 3B | Pager 事务管理测试 | 50% | 5d |
| 3C | Jx9 VM 基础指令测试 | 40% | 5d |
| 3D | Jx9 内置函数测试 | 30% | 5d |
| 3E | API 边界条件测试 | 50% | 3d |
| 3F | 内存安全专项测试 | ASan 运行无报错 | 3d |

**测试优先级排序**:
1. **KV CRUD 操作**（最基础的功能）
2. **事务 ACID 特性**（数据一致性保证）
3. **Jx9 基本语法**（脚本引擎核心）
4. **文件 VFS**（跨平台基础）
5. **边界条件**（大文件、空数据、并发）

**测试环境矩阵**:
| 平台 | 编译器 | 模式 | Sanitizer |
|------|--------|------|-----------|
| Linux x86_64 | GCC 12+ | Debug | ASan + UBSan |
| Linux x86_64 | Clang 15+ | Debug | ASan + UBSan + MSan |
| Windows x64 | MSVC 2022 | Debug | ASan |
| macOS ARM64 | Clang 15+ | Debug | ASan |

---

### Phase 4: 长期改进（第 9-12 周）

| 任务 | 负责人 | 预估工时 |
|------|--------|---------|
| 清理遗留平台代码（WinCE、Borland） | 角色 A | 2d |
| 线程安全改进与默认启用 | 角色 A + B | 3d |
| API 文档完善（Doxygen） | 角色 A | 3d |
| 架构设计文档 | 角色 A | 3d |
| 开发者贡献指南 | 角色 A + C | 1d |
| 性能基准测试套件 | 角色 C | 3d |

---

## 三、协作流程

### 3.1 代码审查要求

| 规则 | 说明 |
|------|------|
| 所有 PR 必须 Code Review | 至少 1 人审核 |
| 安全相关修改 | 角色 A 必须审核 |
| 测试覆盖率 | 新代码 ≥ 80%，修改代码不降低覆盖率 |
| 审查重点 | 内存安全、整数溢出、错误路径、资源释放 |

### 3.2 周会节奏

| 会议 | 频率 | 参与者 | 内容 |
|------|------|--------|------|
| 站会 | 每日 15min | 全员 | 进度同步、阻塞问题 |
| 技术评审 | 每周 1h | 全员 | 代码审查、架构讨论 |
| 回顾会 | 每两周 30min | 全员 | 总结成果、调整计划 |

### 3.3 Git 分支策略

```
main (稳定版本)
  └── develop (开发主线)
       ├── feature/cmake-improvements
       ├── feature/safe-alloc
       ├── feature/split-jx9-builtin
       ├── feature/test-framework
       └── fix/64bit-truncation
```

---

## 四、成功指标

### 4.1 量化目标

| 指标 | 当前状态 | 8 周目标 | 12 周目标 |
|------|---------|---------|----------|
| 编译器警告数 | 未测量 | 0 (Werror) | 0 |
| 测试用例数 | ~0 | 50+ | 150+ |
| 测试覆盖率 | ~0% | 30% | 60%+ |
| ASan 运行 | 未运行 | 无报错 | 无报错 |
| 超大文件 (>3000行) | 7 个 | 4 个 | 0 个 |
| 未修复 TODO/FIXME | 4 个 | 2 个 | 0 个 |
| NULL 检查覆盖 | 未知 | 100% | 100% |
| CI 通过率 | N/A | 100% | 100% |

### 4.2 质量门槛

每个 Phase 完成时需要通过以下检查：
- [ ] 所有新增/修改代码编译零警告
- [ ] 所有测试通过（包括 ASan 构建）
- [ ] 代码审查完成并批准
- [ ] 对应测试覆盖率达标
- [ ] 无新增 TODO/FIXME

---

## 五、工具链推荐

| 类别 | 工具 | 用途 |
|------|------|------|
| 构建系统 | CMake 3.10+ | 已有，需改进 |
| 测试框架 | Unity + Ceedling | C 语言单元测试 |
| 静态分析 | cppcheck | 开源 C 静态分析 |
| 静态分析 | clang-tidy | LLVM 代码检查 |
| 内存检测 | AddressSanitizer | 运行时内存错误检测 |
| 内存检测 | Valgrind (Linux) | 内存泄漏检测 |
| 代码格式 | clang-format | 统一代码风格 |
| CI/CD | GitHub Actions | 自动化构建和测试 |
| 文档 | Doxygen | API 文档生成 |
| 基准测试 | criterion | C 语言性能测试 |

---

## 六、风险与应对

| 风险 | 可能性 | 影响 | 应对措施 |
|------|--------|------|---------|
| 大文件拆分引入新 Bug | 高 | 高 | 充分的回归测试 + 渐进式拆分 |
| 团队成员 C 语言经验不足 | 中 | 高 | Phase 0 以阅读和审查为主，逐步深入 |
| 测试框架选型困难 | 低 | 中 | 先用最简单的 Unity 快速起步 |
| 重构影响向后兼容性 | 中 | 高 | 拆分仅影响内部，公共 API 不变 |
| 进度延迟 | 中 | 中 | Phase 1-2 可并行，关键路径在 Phase 0 |

---

*方案制定: WorkBuddy AI Team Planner*  
*基于: UnQLite 代码审计报告 (2026-04-20)*
