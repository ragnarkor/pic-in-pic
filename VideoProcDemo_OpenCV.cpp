// VideoProcDemo_OpenCV.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "VideoProcDemo_OpenCV.h"
#include "shobjidl_core.h"
#include <windowsx.h>

#include "opencv2/opencv.hpp" 

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

int draw_mode = 0;                              // 0代表不操作，1代表画线，2代表拖动
bool mouse_L_pressed = false;                   // 鼠标左键是否处于被按下的状态
int cur_x=300, cur_y=0;                         // 画中画左上角
int rel_x, rel_y;                               // 相对位置
int offset_x, offset_y;                         // 偏移位置
int new_x, new_y;                               // 新位置
int x, y;                                       // 获取鼠标当前位置

cv::Mat mImg,roi;
WCHAR FileNameOfVideo1[1024];                   // 视频1的文件路径和文件名
cv::VideoCapture VidCap1;                       // 视频1的读取器
WCHAR* mFN = (WCHAR*)FileNameOfVideo1;

cv::Mat sImg;
WCHAR FileNameOfVideo2[1024];                   // 视频2的文件路径和文件名
cv::VideoCapture VidCap2;                       // 视频2的读取器
WCHAR* sFN = (WCHAR*)FileNameOfVideo2;

enum PlayState
{
    playing, paused, stopped
};

PlayState mPlayState = PlayState::stopped;       // 视频1播放状态     
PlayState sPlayState = PlayState::stopped;       // 视频2播放状态 

enum VideoEffect
{
    no, edge
};
VideoEffect mVidEffect = VideoEffect::no;        // 视频1画面效果
VideoEffect sVidEffect = VideoEffect::no;        // 视频2画面效果

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
bool OpenVideoFile(HWND hWnd, LPWSTR* fn);
std::string WCHAR2String(LPCWSTR pwszSrc);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。
    //img = cv::imread("d://wallpaper.jpg");

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_VIDEOPROCDEMOOPENCV, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VIDEOPROCDEMOOPENCV));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIDEOPROCDEMOOPENCV));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_VIDEOPROCDEMOOPENCV);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 500, 500, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   SetTimer(hWnd, 1, 40, NULL);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    bool result;

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_OPEN_VID1:                
                result = OpenVideoFile(hWnd, &mFN);
                if (result){
                    bool opened = VidCap1.open(WCHAR2String(mFN));
                    if (opened){
                        VidCap1 >> mImg;                    // 获取第一帧图像并显示
                        InvalidateRect(hWnd, NULL, false);  // 激发WM_PAINT事件，让窗口重绘
                    }
                    else{
                        MessageBox(
                            hWnd,
                            L"视频未能打开",
                            L"错误提示",
                            MB_OK
                        );
                    }
                }
                break;
            case IDM_PLAY_VID:
                mPlayState = PlayState::playing;
                break;
            case IDM_PAUSE_VID:
                mPlayState = PlayState::paused;
                break;
            case IDM_STOP_VID:
                mPlayState = PlayState::stopped;
                VidCap1.set(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES, 0);                
                break;
            case IDM_NO_EFFECT:                     // 视频1无效果
                mVidEffect = VideoEffect::no;
                break;
            case IDM_EDGE_EFFECT:                   // 视频1边缘图效果
                mVidEffect = VideoEffect::edge;
                break;
            case IDM_NO_EFFECT2:                    // 视频2无效果
                sVidEffect = VideoEffect::no;
                break;
            case IDM_EDGE_EFFECT2:                  // 视频2边缘图效果
                sVidEffect = VideoEffect::edge;
                break;

            case IDM_OPEN_VID2:
                result = OpenVideoFile(hWnd, &sFN);
                if (result)
                {
                    //img = cv::imread(WCHAR2String(fn));
                    bool opened = VidCap2.open(WCHAR2String(sFN));

                    if (opened)
                    {
                        VidCap2 >> sImg;    // 获取第一帧图像并显示
                        InvalidateRect(hWnd, NULL, false);  // 激发WM_PAINT时间，让窗口重绘
                    }
                    else
                    {
                        MessageBox(
                            hWnd,
                            L"视频未能打开",
                            L"错误提示",
                            MB_OK
                        );
                    }
                }
                break;
            case IDM_PLAY_VID2:
                sPlayState = PlayState::playing;
                break;
            case IDM_PAUSE_VID2:
                sPlayState = PlayState::paused;
                break;
            case IDM_STOP_VID2:
                sPlayState = PlayState::stopped;
                VidCap2.set(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES, 0);
                break;

            case IDM_START_DRAW_LINE:
                if (mImg.rows > 0 || sImg.rows > 0)
                    draw_mode = 1;
                break;
            case IDM_STOP_DRAW_LINE:
                draw_mode = 0;
                break;

            case IDM_START_DRAW_WIN:
                if (sImg.rows > 0)
                    draw_mode = 2;
                break;
            case IDM_STOP_DRAW_WIN:
                draw_mode = 0;
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_TIMER:
        if (VidCap1.isOpened() && mPlayState == PlayState::playing)
        {
            VidCap1 >> mImg;
            // 循环播放设置
            if (mImg.empty())
            {
                VidCap1.open(WCHAR2String(mFN));
                VidCap1 >> mImg; //重新获取第一帧图像并显示
            }
            // 边缘图滤镜设置
            if (mVidEffect == VideoEffect::edge)
            {
                cv::Mat edgeY, edgeX;                   // 保存x,y方向的边缘
                cv::Sobel(mImg, edgeY, CV_8U, 1, 0);    // 获取x方向边缘
                cv::Sobel(mImg, edgeX, CV_8U, 0, 1);    // 获取y方向边缘
                mImg = edgeX + edgeY;                   // 合成x,y方向边缘图
            }
            InvalidateRect(hWnd, NULL, false);
        }

        if (VidCap2.isOpened() && sPlayState == PlayState::playing)
        {
            VidCap2 >> sImg;
            // 循环播放设置
            if (sImg.empty())
            {
                VidCap2.open(WCHAR2String(sFN));
                VidCap2 >> sImg; //获取第一帧图像并显示
            }
            // 边缘图滤镜设置
            if (sVidEffect == VideoEffect::edge)
            {
                cv::Mat edgeY, edgeX;
                cv::Sobel(sImg, edgeY, CV_8U, 1, 0);
                cv::Sobel(sImg, edgeX, CV_8U, 0, 1);
                sImg = edgeX + edgeY;
            }
            // 画中画效果调整
            if (mImg.rows > 0) {
                resize(mImg, mImg, cv::Size(500, 500));
                resize(sImg, sImg, cv::Size(200, 200));
                roi = mImg(cv::Rect(cur_x, cur_y, 200, 200));   // 设置主视频的ROI区域
                sImg.copyTo(roi);
            }
            InvalidateRect(hWnd, NULL, false);
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // 绘制主视频
        if (mImg.rows > 0)
        {
            resize(mImg, mImg, cv::Size(500, 500));
            switch (mImg.channels())
            {
            case 1:
                cv::cvtColor(mImg, mImg, cv::COLOR_GRAY2BGR); // GRAY单通道
                break;
            case 3:
                cv::cvtColor(mImg, mImg, cv::COLOR_BGR2BGRA);  // BGR三通道
                break;
            default:
                break;
            }
            int pixelBytes = mImg.channels() * (mImg.depth() + 1); // 计算一个像素多少个字节

            // 制作bitmapinfo(数据头)
            BITMAPINFO bitInfo;
            bitInfo.bmiHeader.biBitCount = 8 * pixelBytes;
            bitInfo.bmiHeader.biWidth = mImg.cols;
            bitInfo.bmiHeader.biHeight = -mImg.rows;
            bitInfo.bmiHeader.biPlanes = 1;
            bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bitInfo.bmiHeader.biCompression = BI_RGB;
            bitInfo.bmiHeader.biClrImportant = 0;
            bitInfo.bmiHeader.biClrUsed = 0;
            bitInfo.bmiHeader.biSizeImage = 0;
            bitInfo.bmiHeader.biXPelsPerMeter = 0;
            bitInfo.bmiHeader.biYPelsPerMeter = 0;

            StretchDIBits(
                hdc,
                0, 0, 500, 500,
                0, 0, mImg.cols, mImg.rows,
                mImg.data,
                &bitInfo,
                DIB_RGB_COLORS,
                SRCCOPY
            );
        }
        // 绘制画中画视频
        if (sImg.rows > 0)
        {
            resize(sImg, sImg, cv::Size(200, 200));
            switch (sImg.channels())
            {
            case 1:
                cv::cvtColor(sImg, sImg, cv::COLOR_GRAY2BGR); // GRAY单通道
                break;
            case 3:
                cv::cvtColor(sImg, sImg, cv::COLOR_BGR2BGRA);  // BGR三通道
                break;
            default:
                break;
            }

            int pixelBytes = sImg.channels() * (sImg.depth() + 1); // 计算一个像素多少个字节

                                                                 // 制作bitmapinfo(数据头)
            BITMAPINFO bitInfo;
            bitInfo.bmiHeader.biBitCount = 8 * pixelBytes;
            bitInfo.bmiHeader.biWidth = sImg.cols;
            bitInfo.bmiHeader.biHeight = -sImg.rows;
            bitInfo.bmiHeader.biPlanes = 1;
            bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bitInfo.bmiHeader.biCompression = BI_RGB;
            bitInfo.bmiHeader.biClrImportant = 0;
            bitInfo.bmiHeader.biClrUsed = 0;
            bitInfo.bmiHeader.biSizeImage = 0;
            bitInfo.bmiHeader.biXPelsPerMeter = 0;
            bitInfo.bmiHeader.biYPelsPerMeter = 0;
            // Mat.data + bitmap数据头 -> MFC

            StretchDIBits(
                hdc,
                cur_x, cur_y, 200, 200,
                0, 0, sImg.cols, sImg.rows,
                sImg.data,
                &bitInfo,
                DIB_RGB_COLORS,
                SRCCOPY
            );
        }
        EndPaint(hWnd, &ps);
    }
        break;

    case WM_LBUTTONDOWN: {
        mouse_L_pressed = true;
        x = GET_X_LPARAM(lParam);
        y = GET_Y_LPARAM(lParam);
        rel_x = x - cur_x;
        rel_y = y - cur_y;
    }
        break;
    case WM_LBUTTONUP:
        mouse_L_pressed = false;
        break;

    case WM_MOUSEMOVE:
        x = GET_X_LPARAM(lParam);
        y = GET_Y_LPARAM(lParam);
        // 绘制图像
        if (mouse_L_pressed && draw_mode == 1)
        {
            if (sImg.rows > 0) {                    // 在画中画中绘画
                cv::Point p(rel_x, rel_y);
                cv::circle(sImg, p, 8, cv::Scalar(0, 0, 255), -1);
            }
            if (mImg.rows > 0) {                    // 在主视频中绘画
                cv::Point p(x, y);
                cv::circle(mImg, p, 8, cv::Scalar(0, 0, 255), -1);
            }
        }
        // 拖动窗口
        else if (mouse_L_pressed && draw_mode == 2){ 
            if (0 <= rel_x && rel_x <= 200
                && 0 <= rel_y && rel_y <= 200) { //判断鼠标是否位于画中画区域

                offset_x = (cur_x + rel_x) - x;
                offset_y = (cur_y + rel_y) - y;
                new_x = cur_x - offset_x;
                new_y = cur_y - offset_y;

                if (0 <= new_x && new_x <= 300
                    && 0 <= new_y && new_y <= 300) {    // 判断新的画中画位置是否超出原视频
                    cur_x = new_x;
                    cur_y = new_y;
                }
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

bool OpenVideoFile(HWND hWnd, LPWSTR* fn){
    IFileDialog* pfd = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pfd));

    DWORD dwFlags;
    hr = pfd->GetOptions(&dwFlags);
    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);

    COMDLG_FILTERSPEC rgSpec[] ={
        { L"MP4", L"*.mp4" },
        { L"AVI", L"*.avi" },
        { L"ALL", L"*.*" },};

    HRESULT SetFileTypes(UINT cFileTypes, const COMDLG_FILTERSPEC * rgFilterSpec);
    hr = pfd->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);
    hr = pfd->SetFileTypeIndex(1);

    hr = pfd->Show(hWnd);   //显示打开文件对话框
    IShellItem* pShellItem = NULL;
    if (SUCCEEDED(hr)){
        hr = pfd->GetResult(&pShellItem);
        hr = pShellItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, fn);  //获取文件的完整路径
        return true;
    }
    return false;
}

std::string WCHAR2String(LPCWSTR pwszSrc)
{
    int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);
    if (nLen <= 0)
        return std::string("");

    char* pszDst = new char[nLen];
    if (NULL == pszDst)
        return std::string("");

    WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
    pszDst[nLen - 1] = 0;

    std::string strTmp(pszDst);
    delete[] pszDst;

    return strTmp;
}

//————————————————
//版权声明：本文为CSDN博主「kingkee」的原创文章，遵循CC 4.0 BY - SA版权协议，转载请附上原文出处链接及本声明。
//原文链接：https ://blog.csdn.net/kingkee/java/article/details/98115024