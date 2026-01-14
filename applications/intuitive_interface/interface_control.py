#界面控制
#更新 control_array = [scale_factor, horizontal_position, vertical_position]

import pygame
import sys

displacement=[0,0] #维护一个位置变量
disp=15 #调节移动速度
def data_process(data_values):
    high=1500000
    low=800000
    scale_factor=(data_values[2]-low)/(high-low)
    if scale_factor<=0.1:
        scale_factor=0.1
    if scale_factor>=0.8:
        scale_factor=0.8
    scale_factor=1-scale_factor
    x=data_values[8]#左为负，右为正
    y=data_values[7]#下为负，上为正
    if x<-30:
        displacement[0]=displacement[0]-disp
    if x>30:
        displacement[0]=displacement[0]+disp
    if y<-25:
        displacement[1]=displacement[1]+disp
    if y>25:
        displacement[1]=displacement[1]-disp

    control_array = [scale_factor, displacement[0], displacement[1]]
    return control_array

def init_interface():
    # 初始化Pygame
    pygame.init()

    # 获取显示设备的宽度和高度
    width, height = pygame.display.Info().current_w, pygame.display.Info().current_h
    # 设置窗口大小为全屏
    screen = pygame.display.set_mode((width, height), pygame.FULLSCREEN)
    #print(width, height)
    # 加载图像
    image = pygame.image.load("D:/0-科研/1-人机交互手套/5-人机交互应用/界面控制/清华LOGO.png")

    return screen,image

def interface_update(control_array,screen,image,running = True):
    #control_array = [scale_factor, horizontal_position, vertical_position]
    #scale_factor = 0.5  # 初始缩放因子
    #horizontal_position = 0  # 初始水平偏移
    #vertical_position = 0  # 初始垂直偏移
    
    # 获取显示设备的宽度和高度
    width, height = pygame.display.Info().current_w, pygame.display.Info().current_h
    image_rect = image.get_rect()
    position=image_rect
    # 游戏循环
    if running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False
        # 清屏
        screen.fill((255, 255, 255))

        # 设置缩放后的图像宽度和高度
        scaled_width = image.get_width() * control_array[0]
        scaled_height = image.get_height() * control_array[0]

        # 调整图像大小
        scaled_image = pygame.transform.smoothscale(image, (scaled_width, scaled_height))
        
        position=image_rect
        # 更新图标的位置，以屏幕中心为基准
        position.left = int((width-scaled_width)/2)+control_array[1]
        position.top = int((height-scaled_height)/2)+control_array[2]
        #print(position.left,position.top)
        # 将图像绘制到窗口
        screen.blit(scaled_image, position)
        # 更新窗口显示
        pygame.display.flip()
        # 延时200毫秒
        #pygame.time.delay(20)
    # 退出Pygame
    #pygame.quit()
    #sys.exit()
