主要思路就是利用mutex实现深层次copy原子性。并且要用malloc
当然这也可能带来内存泄漏的问题。

我的代码是参考 https://dreamanddead.github.io/CSAPP-3e-Solutions/chapter12/12.26/
主要也就是对他的代码进行了优化 和添加了测试
