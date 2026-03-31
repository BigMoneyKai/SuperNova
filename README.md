
| memtag                                      | 分配器类型                              | 用途              |
| ------------------------------------------- | ---------------------------------- | --------------- |
| MEM_TAG_ARRAY / MEM_TAG_DARRAY              | pool_allocator                     | 小固定大小数组元素       |
| MEM_TAG_STRING                              | linear_allocator / stack_allocator | 短生命周期字符串缓冲      |
| MEM_TAG_TEXTURE / MEM_TAG_MATERIAL_INSTANCE | general allocator                  | GPU/资源对象，生命周期复杂 |
| MEM_TAG_JOB                                 | stack_allocator                    | 临时任务内存，帧内释放     |
| MEM_TAG_SCENE / MEM_TAG_ENTITY_NODE         | general allocator / linear         | 生命周期长，需按需释放     |
