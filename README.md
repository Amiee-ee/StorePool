# StorePool

## 项目简介

`StorePool` 是一个简单的等大小内存池实现，演示如何通过预分配大块内存并切分为固定小块来提高频繁分配/释放小对象的性能。该项目包含一个 `StorePool` 类和一个示例 `Person` 类型，通过重载 `operator new` / `operator delete` 演示内存池用法。

## 主要特性

- 等大小块内存池（按块数和块大小分配一次性内存页）
- 内存对齐处理以保证指针对齐安全
- 释放时将块回收到空闲链表中以实现重用
- 支持在类型内部重载 `new`/`delete` 以透明使用内存池
- 仅依赖标准 C++14，易于移植与嵌入到现有项目

## 文件结构

- `main.cpp` - 演示用例和 `StorePool` 的实现（当前活动文件）
- `StorePool.vcxproj.filters` - Visual Studio 过滤器文件

## 构建与运行（Visual Studio 2022）

1. 在 Visual Studio 中打开解决方案或 `StorePool` 文件夹。
2. 选择配置（Debug/Release）和目标平台（x86/x64）。
3. 构建并运行。

或者使用命令行（已安装 MSVC）：
- 使用 `cl` / `msbuild` 按照常规方式构建。

## 使用示例

示例代码位于 `main.cpp`：

- 全局静态 `StorePool sPerson(1024, sizeof(Person));` 为 `Person` 类分配内存页，每页 1024 个固定大小块。
- 在 `Person` 内部重载 `operator new` 和 `operator delete`，将 `new`/`delete` 的行为重定向到 `sPerson.allocate()` / `sPerson.deallocate()`。
- `main` 演示了批量分配与释放、以及释放后内存重用的验证。

运行后可观察到对象分配与释放，并验证内存块被复用。

## API 概览

- `explicit StorePool(size_t blocks_number, size_t blocks_size);`
  - 构造函数，指定每页块数量与每块大小（内部会做对齐与最小尺寸调整）。

- `~StorePool();`
  - 析构函数，释放所有向系统申请的页面内存。

- `void* allocate();`
  - 从空闲链表取出一个块并返回指针；若链表为空则调用 `expand()` 分配新页面。

- `void deallocate(void* p);`
  - 将块返回到空闲链表以供重用；如果传入 `nullptr` 则无操作。

- `size_t blocks_number() const;` / `size_t blocks_size() const;`
  - 访问构造时的块数与（调整后的）块大小。

## 设计要点

- 内存对齐：使用 `align_up` 保证每个块满足 `alignof(void*)` 的对齐要求，避免未定义行为。
- 页面管理：每次 `expand()` 按 `blocks_number * blocks_size` 申请一页连续内存，并把每块连接到空闲链表。
- 空闲链表：使用块本身的内存作为链表结点（`ListNode`），无额外元数据开销。
- 安全性与异常：`operator delete` 被声明为 `noexcept`，析构时统一释放页面，避免内存泄漏。

## 常见改进方向

- 支持可变块大小或多级池（small/medium/large）以处理不同大小对象。
- 添加线程安全（互斥或无锁）以支持并发分配/释放。
- 使用按需释放或回收空闲页面以减少内存占用。
- 将页面持久化或与自定义分配器集成用于特殊平台。

## 测试与基准

- 当前仓库包含一个基本示例用于验证功能。可扩展为单元测试（例如使用 Catch2 或 Google Test）覆盖分配、释放、边界条件与并发场景。
- 基准测试关注点：分配/释放吞吐量、内存占用、重用效率。

## 贡献

欢迎提交 Issue、Pull Request 或改进建议：
- 新特性（线程安全、多池策略）
- 更完善的示例或基准
- 单元测试和 CI 配置

## 许可证

请在仓库中添加适当 LICENSE（如 MIT）并在合并前确认许可类型。

---
