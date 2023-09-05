/**
 * 基于GEC6818开发板的打地鼠游戏
*/
#include <stdio.h>
#include "whack_mole.h"
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <linux/input.h>
#include <time.h>
#include "thread_pool.h"

int arr[3][3]={0};//一个3*3数组，保存地洞数据,全部初始化为0
int lcd_id=-1;//LCD屏幕文件描述符,初始化为-1
struct fb_var_screeninfo lcd_info;//获取屏幕属性数据，未初始化
int *lcd_mmap=NULL;//LCD屏幕文件映射，初始化为NULL
int x,y;//触碰坐标，不初始化
int outx,outy;//点击坐标，不初始化
int score=0;//游戏得分，初始化为0
int hp=0;//生命值，初始化为0


void LCD_Init()
{
    /**
     * 打开LCD屏幕文件，给予读写权限
    */
    lcd_id=open(LCD_PATH,O_RDWR);
    if(lcd_id==-1)
    {
        perror("open LCD_file failed\n");
        return ;
    }
    /**
     * 获取屏幕文件属性数据
     */
    ioctl(lcd_id,FBIOGET_VSCREENINFO,&lcd_info);
    int lcd_size=lcd_info.xres*lcd_info.yres*(lcd_info.bits_per_pixel / 8);//算出屏幕文件大小
    /**
     * 映射文件
    */
    lcd_mmap=mmap(NULL,lcd_size,PROT_READ | PROT_WRITE,MAP_SHARED,lcd_id,0);
    if(lcd_mmap==MAP_FAILED)
    {
        perror("creat mmap_file failed\n");
        munmap(lcd_mmap,800*480*4);//收尾工作
        close(lcd_id);
        lcd_id=-1;
        return ;
    }
}

void BMP_Open(int x,int y,const char *bmppath)
{
    //打开bmp文件
	int bmp_id=open(bmppath,O_RDONLY);
	if(bmp_id==-1)
	{
		perror("");
		exit(0);
	}
    // 定义一个4字节的空间去存储读取到的数据

    unsigned char data[4]={0};

    read(bmp_id,data,2);

    // 判断是否为BMP图片
    if(data[0]!= 0x42 || data[1] != 0x4d)
    {
        puts("this picture not bmp file!");
        return;
    }

    // 读取像素数组的偏移量数据
    lseek(bmp_id,0x0a,SEEK_SET);
    read(bmp_id,data,4);
    // 数据还原，取决大小端模式
    int offset = data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0];

    // 读取图片宽度
    lseek(bmp_id,0x12,SEEK_SET);
    read(bmp_id,data,4);
    // 数据还原，取决大小端模式
    int width = data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0];

    // 读取图片高度
    read(bmp_id,data,4);
    // 数据还原，取决大小端模式
    int height = data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0];

    // 读取色深
    lseek(bmp_id,0x1c,SEEK_SET);
    read(bmp_id,data,2);
    // 数据还原，取决大小端模式
    int depth = data[1] << 8 | data[0];

    printf("weight:%d,height:%d,depth:%d\n",width,height,depth);

    // 偏移到像素数组的位置
    lseek(bmp_id,offset,SEEK_SET);

    // 计算填充字节数
    int fills = 0;

    if((width*depth/8) % 4)
    {
        fills = 4 - (width*depth/8) % 4 ;
    }

    // 实际一行的字节数
    int real_width = width*depth/8 + fills;

    // 计算出像素数组的大小
    int pixel_array_size = real_width*abs(height);

    // 获取像素数组的数据
    unsigned char *color_point = (unsigned char*)malloc(pixel_array_size);
	int i=0;
    read(bmp_id,color_point,pixel_array_size);
    // 显示图片
    for(int h = 0;h < abs(height);h++)
    {
        for(int w = 0;w < width;w++)
        {
            unsigned char a,r,g,b;
            b = color_point[i++];
            g = color_point[i++];
            r = color_point[i++];
            a = depth == 24?0:color_point[i++];

            // 整合颜色
            int color = a << 24|r << 16|g << 8|b;
            LCD_Darw(w+x,((height < 0)?h:(height-1-h))+y,color);
        }
        // 每一行结束跳过填充字节
        i+=fills;
    }
	free(color_point);
	close(bmp_id);
}

void LCD_Darw(int x,int y,int color)
{
    if(x>=0 && x<lcd_info.xres && y>=0 && y<lcd_info.yres)
    {
        *(lcd_mmap+x+y*lcd_info.xres)=color;
        return ;
    }
    else
    {
        printf("param is error\n");
        return ;
    }
}

void Input_Event()
{
    FILE *event_id=fopen(INPUT_PATH,"r+");//打开输入子事件文件
    if(event_id==NULL)
    {
        perror("open input_file failed");
        return ;
    }
    struct input_event ev;//获取输入子事件
    /**
     * 获取点击事件
    */
    while(1)
    {
        int size=fread(&ev,sizeof(ev),1,event_id);
        if(size!=1)
        {
            continue;
        }
        if(ev.type == EV_ABS) // 绝对事件
        {
            // 获取X轴的数值
            if(ev.code == ABS_X)
            {
                x = ev.value*0.78;
            }
        }
        if(ev.type == EV_ABS) // 绝对事件
        {
            // 获取X轴的数值
            if(ev.code == ABS_Y)
            {
                y = ev.value*0.78;
            }
        }
        // 判断压感
        if(ev.type == EV_KEY && ev.code == BTN_TOUCH && ev.value == 0)
        {
            outx=x;
			outy=y;
            fclose(event_id);
			return;
		}
    }
}

void Main_Menu()
{
    /**
     * 死循环主界面，直到选择退出程序
    */
    while(1)
    {
        BMP_Open(0,0,MENU_PATH);//绘制主界面画面
        Input_Event();//获取输入子事件
        /**
         * 根据不同条件判断进入哪个界面
        */
       if(outy>=381 && outy<=413)
       {
            if(outx>=200 && outx<=310)
            {
                Game_Menu();
            }
            else if(outx>=342 && outx<=454)
            {
                Rank_List();
            }
            else if(outx>=489 && outx<=601)
            {
                Exit_Game();
            }
       }
    }
    return ;
}

void Game_Menu()
{
    score=0;//得分初始化为0
    hp=10;//生命值初始化为10
    /**
     * 绘制游戏初始界面
    */
    BMP_Open(0,0,BACK_PATH);
    for(int i=0;i<3;i++)
    {
        for(int j=0;j<3;j++)
        {
            BMP_Open(136+j*170,66+i*130,NOMOLE_PATH);
        }
    }
    thread_pool_t *pool = thread_pool_init(7);//创建一个最多有7个线程的线程池
    add_new_task(pool,NULL,routine);//添加一次地鼠任务线程
    add_new_task(pool,NULL,routine2);//添加一次获取输入子事件任务线程
    while(1)
    {
        if(score!=0 && score <=100 && score %20==0)
        {
            add_new_task(pool,NULL,routine);//地鼠任务线程逐级增加，最大为6
            sleep(3);
        }
        if(hp==0)
        {
            thread_destroy(&pool);//生命值为0，销毁线程池，停止线程
            Rank_Write(score);
            BMP_Open(200,120,OVER_PATH);
            sleep(1);
            return;
        }
    }
    

}

int Game_Judgment()
{
    if(hp==0)
     return 0;
    else
     return 1;
}

void *routine(void *args)
{
    
    while(1)
    {   
        /**
        * 让系统在0-3随机生成数,生成出地鼠
        */
        int col,row;
        srand((unsigned)time(NULL));
        col=rand()%3;
        row=rand()%3;
        if(arr[col][row]!=0)
        {
            continue;
        }
        arr[col][row]=1;
        BMP_Open(136+row*170,66+col*130,MOLE_PATH);//绘制地鼠
        /**
         * 将坐标重新初始化，避免读取上一次残留数据
        */
        outx=-1;
        outy=-1;
        /**
         * 定时循环，判断用户是否打到地鼠
        */
        time_t start_time, current_time;
        int timeout = 2; // 设置超时时间为2秒
        time(&start_time); // 记录循环开始的时间
        while (1) 
        {
            
            if(outx>=(135+row*170) && outx<=(290+row*170) && outy>=(66+col*130) && outy<=(172+col*130))
            {
                score++;//打到地鼠，得分加一
                printf("score=%d\n",score);
                break;
            }
            time(&current_time); // 获取当前时间
            // 判断当前时间与开始时间的差是否超过超时时间
            if (current_time - start_time >= timeout) // 超时退出循环
            {
                hp--;//否则生命值减一
                printf("hp is %d\n",hp);
                break; // 超时退出循环
            }
        }
        //还原数据
        arr[col][row]=0;
        //还原游戏背景
        BMP_Open(136+row*170,66+col*130,NOMOLE_PATH);
        sleep(1);
    }
}

void *routine2(void *args)
{
    /**
     * 循环获取输入子事件
    */
    while(1)
    {
        Input_Event();
    }
}

void Rank_List()
{
    int Rank_arr[10]={0};
    int flag=0;
    int rank_id=open(RANK_PATH,O_RDWR);
    if(rank_id==-1)
    {
        perror("open rank.txt failed\n");
        return;
    }
    while(read(rank_id,&Rank_arr[flag],4)==4 && flag<10)//读取文件内容
    {
        flag++;
    }
    /**
     * 冒泡排序
    */
    if(flag==0)
    {
        printf("NOT PALY GAME\n");
        return ;
    }
    for(int i=0;i<10;i++)
    {
        for(int j=i+1;j<10;j++)
        {
            if(Rank_arr[j]>=Rank_arr[i])
            {
                Rank_arr[j]=Rank_arr[j]^Rank_arr[i];
                Rank_arr[i]=Rank_arr[j]^Rank_arr[i];
                Rank_arr[j]=Rank_arr[j]^Rank_arr[i];
            }
        }
    }
    for(int i=0;i<10;i++)//打印排行榜
    {
        printf("NO.%d:%d\n",i+1,Rank_arr[i]);
    }
    close(rank_id);
    return;
}

void Exit_Game() 
{
    close(lcd_id);
    munmap(lcd_mmap,800*480*4);
    exit(0);
}

void Rank_Write(int num)
{
    /**
     * 打开rank.txt文件并且将分数写入到文件里
    */
    int rank_id=open(RANK_PATH,O_RDWR | O_APPEND);
    if(rank_id==-1)
    {
        perror("open rank.txt failed\n");
        return;
    }
    int flag=write(rank_id,&num,sizeof(num));
    if(flag==-1)
    {
        perror("write rank.txt failed\n");
        close(rank_id);
        return ;
    }
    close(rank_id);
    return ;
}