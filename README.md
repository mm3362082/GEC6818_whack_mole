# 项目名称：

```
基于GEC6818欢乐打地鼠游戏
```

# 项目开发时间：

```
第一版：2023年8月30日-2023年8月31日
```

# 项目需求：

```
实现打地鼠的基础游戏功能，排行榜功能，退出功能
```

# 项目关键词：

```
文件IO，多线程，C语言，数据结构
```

# 项目文件：

```
whack_mole
```

# 子文件夹及其子文件：

```
photo:

- background.bmp

- gameover.bmp

- menu.bmp

- mole.bmp

- nomole.bmp

  project:

- main.c

- program

- rank.txt

- thread_pool.c

- thread_pool.h

- whack_mole.c

- whack_mole.h
						
```

# 项目大致框架：

```
	（1）GEC6818开发板LCD屏幕的初始化
	（2）读取BMP图片文件信息，并显示在LCD屏幕上
	（3）获取GEC6818开发板输入子事件
	（4）判断输入子事件，实现点击功能
	（5）打地鼠的基础玩法，多线程实现地鼠的自主出现和消失，记录分数和游戏结束条件判断
	（6）主菜单界面
	（7）排行榜界面
	（8）程序退出
```

# 项目程序和函数模块：

## （1）项目源码文件：

```
main.c:主函数，实现屏幕的初始化和主界面的加载
thread_pool.c:实现线程池
thread_pool.h:线程池的头文件
whack_mole.c:打地鼠游戏的功能实现
whack_mole.h:打地鼠游戏功能头文件
program：打地鼠游戏可执行程序（开发板）
```

## （2）函数模块：

### 	1.whack_mole.c函数模块

```c
/**
 *@brief 屏幕初始化
 *@return void 
*/
void LCD_Init();

/**
 *@brief 打开BMP图片并且将BMP图片进行绘制
 *@param x BMP图片x轴偏移量
 *@param y BMP图片y轴偏移量
 *@param bmppath BMP图片的存放位置
 *@return void
*/
void BMP_Open(int x,int y,const char *bmppath);

/**
 * @brief 像素点绘制
 * @param x x轴偏移量
 * @param y y轴偏移量
 * @param color 绘制颜色
 * @return void
*/
void LCD_Darw(int x,int y,int color);

/**
 * @brief 获取输入子事件
 * @return void
*/
void Input_Event();

/**
 * @brief 主菜单界面
 * @return void
*/
void Main_Menu();

/**
 * @brief 游戏界面
 * @return void
*/
void Game_Menu();

/**
 * @brief 判断失败模块
 * @return int,失败返回0，否则返回1
*/
int Game_Judgment();

/**
 * @brief 排行榜界面
 * @param num 需要写入的数据
 * @return void
*/
void Rank_List();

/**
 * @brief 退出模块
 * @return void
*/
void Exit_Game();

/**
 * @brief 地鼠任务函数
 * @param args
 * @return void
*/
void *routine(void *args);

/**
 * @brief 获取输入子事件任务函数
 * @param args
 * @return void
*/
void *routine2(void *args);

/**
 * @brief 分数写入模块
 * @param num 需要写入的分数
 * @return void
*/
void Rank_Write(int num);
```

### 2.thread_pool.c函数模块

```c
typedef void *(*thread_t)(void *);


typedef struct tasks
{
    // 任务指针
    void *(*task_point)(void *);

    // 任务参数
    void *args;

    // 下一个任务
    struct tasks *next;
}task_t;

/**
 * @brief 线程池结构体类型申明
 * 
 * 
 */
typedef struct thread_pool
{
    // 线程个数
    int             thread_count;

    // 标明线程池是否启动
    int             pool_status;

    // 线程id的集合
    pthread_t      *thread_id;

    // 线程共享的互斥锁
    pthread_mutex_t shared_mutex;

    // 线程共享的条件变量
    pthread_cond_t  shared_cond;

    // 任务链表
    task_t         *task_list;

    /*
        最大线程数目：表示可以支持线程并发的最大线程数
        当前服役线程数目：表示当前可以用执行任务的线程
        当前休眠线程数目：表示当前可支持同时并发的线程数
        ...
    */

}thread_pool_t;

/**
 * @brief 线程池初始化
 * @param thread_count 线程池中线程的数量
 * @return thread_pool_t 返回一个初始化的线程池
 */
thread_pool_t *thread_pool_init(int count);

/**
 * @brief 添加任务到任务链表
 * @param pool 需要添加任务的线程池
 * @param args 任务执行时需要的参数
 * @param task_rountine 任务函数指针
 * @return void
 */
void add_new_task(thread_pool_t *pool,void *args,thread_t task_rountine);

/**
 * @brief 销毁线程池
 * @param pool 二级指针 需要销毁的线程池指针
 */
void thread_destroy(thread_pool_t **pool);

/*
    add_task_thread:增加线程数
    del_task_thread:删除线程数
    ...
*/
```

# 项目待开发功能：

```
（1）游戏得分（score）和血量(hp)可视化
（2）游戏界面的暂停和退出功能
（3）排行榜可视化
（4）退出功能插入退出成功图片
```

