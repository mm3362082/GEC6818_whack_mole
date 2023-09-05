#ifndef __WHACK_MOLE_H__
#define __WHCAK_MOLE_H__

#include <linux/fb.h>

/**
 * 声明全局变量
*/
extern int arr[3][3];//一个3*3数组，保存地洞数据,全部初始化为0
extern int lcd_id;//LCD屏幕文件描述符,初始化为-1
extern struct fb_var_screeninfo lcd_info;//获取屏幕属性数据，未初始化
extern int *lcd_mmap;//LCD屏幕文件映射，初始化为NULL
extern int x,y;//触碰坐标，不初始化
extern int outx,outy;//点击坐标，不初始化
extern int score;//游戏得分，初始化为0
extern int hp;//生命值，初始化为0


#define MOLE 1//宏定义老鼠的数据

/**
 * 宏定义文件存放地址
*/
#define LCD_PATH ("/dev/fb0")//LCD屏幕文件
#define INPUT_PATH ("/dev/input/event0")//输入子事件文件
#define RANK_PATH ("/whack_mole/rank.txt")//排行榜文件

/**
 * 宏定义BMP图片文件存放位置
*/
#define MENU_PATH ("/whack_mole/photo/menu.bmp")//主菜单界面图片
#define BACK_PATH ("/whack_mole/photo/background.bmp")//游戏界面背景图片
#define MOLE_PATH ("/whack_mole/photo/mole.bmp")//地鼠图片
#define NOMOLE_PATH ("/whack_mole/photo/nomole.bmp")//无地鼠图片
#define OVER_PATH ("/whack_mole/photo/gameover.bmp")//游戏结束图片
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
#endif