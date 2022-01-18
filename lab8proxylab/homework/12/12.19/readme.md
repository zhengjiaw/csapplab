维护一个 prev_is_write 初始化为0变量
读者每次读完的时候，prev_is_write = 0
写者写完的时候 prev_is_write = 1 ，写者每次开始写，如果readcnt > 0 && prev_is_write 就 什么都不做

写者在 P(mutex) 之前先 P 后 V