# Generate-TruthTable
c语言，用一个.c文件就实现真值表的生成

而己。

直接用法说明：

共六种符号
|~|&|^|\||>|=|
|---|---|---|---|---|---|
|非|与|异或|或|蕴含|等价|

例如以下函数：

```format("%v=%v", "A", "B");```

表示
$$A\Leftrightarrow B$$
其中A和B任意真值指派后的真值表

于是运行结果为
```
Result for FF: 1
Result for TF: 0
Result for FT: 0
Result for TT: 1
```
