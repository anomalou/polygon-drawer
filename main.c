#define UNICODE

#include <windows.h>
#include <windowsx.h>

HINSTANCE hInst;

HWND mainHWND;

struct Node{
    struct Node* next;
    byte allow;
    int x;
    int y;
};

struct Figure{
    struct Node* head;
    struct Figure* next;
    byte nodeNumber;
    byte complete;
    POINT paintPoint;
    byte filled;
};

struct Setup{
    struct Figure* currentFigure;
    struct Node* currentNode;
};

struct Figure* headFigure;

struct Figure* currentPaintFigure;

struct Figure* AddFigure();
struct Node* AddNode(struct Figure* figure);
void InsertNode(struct Figure* figure, struct Node* node);
struct Node* GetLastNode(struct Figure* figure);

WINBOOL Intersect(struct Node* a, struct Node* b, struct Node* c, struct Node* d);
WINBOOL IsVectorIntersect(struct Node* a, struct Node* b);
WINBOOL IsPointInside(int x, int y);
void RecurFill(HDC hdc, int x, int y);

HBRUSH success;
HBRUSH falue;
HBRUSH solid;
HPEN pSuccess;
HPEN pFalue;
HPEN pSolid;

LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE lhInst, LPSTR cmd, INT cmdCount){
    hInst = hInstance;

    WNDCLASS mainClass = {};
    mainClass.hCursor = LoadCursor(NULL, IDC_CROSS);
    mainClass.lpfnWndProc = WinProc;
    mainClass.hInstance = hInstance;
    mainClass.lpszClassName = L"MainClass";
    mainClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&mainClass);

    mainHWND = CreateWindow(L"MainClass", L"GeoGraph", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    ShowWindow(mainHWND, SW_SHOW);

    MSG msg = {};
    while(GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    struct Setup* setup;

    if(msg == WM_CREATE){
        headFigure = NULL;
        setup = malloc(sizeof(struct Setup));
        setup->currentFigure = NULL;
        setup->currentNode = NULL;
        currentPaintFigure = NULL;
        success = CreateSolidBrush(RGB(100, 255, 100));
        falue = CreateSolidBrush(RGB(255, 100, 100));
        solid = CreateSolidBrush(RGB(0, 0, 0));
        pSuccess = CreatePen(PS_SOLID, 4, RGB(100, 255, 100));
        pFalue = CreatePen(PS_SOLID, 4, RGB(255, 100, 100));
        pSolid = CreatePen(PS_SOLID, 4, RGB(0, 0, 0));
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)setup);
    }else{
        setup = (struct Setup*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    switch(msg){
        case WM_ERASEBKGND:
        break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            RECT paintRect;
            GetClientRect(hwnd, &paintRect);
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP bitMap = CreateCompatibleBitmap(hdc, paintRect.right, paintRect.bottom);
            int saveDC = SaveDC(memDC);
            SelectObject(memDC, bitMap);
            FillRect(memDC, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            struct Figure* figure = headFigure;
            while(figure != NULL){
                if(figure->nodeNumber > 0){
                    struct Node* prev = figure->head;
                    struct Node* next = prev->next;
                    SelectObject(memDC, solid);
                    SelectObject(memDC, pSolid);
                    if(next == NULL)
                        Ellipse(memDC, prev->x - 3, prev->y - 3, prev->x + 3, prev->y + 3);
                    while(next != NULL){
                        MoveToEx(memDC, prev->x, prev->y, NULL);
                        LineTo(memDC, next->x, next->y);
                        Ellipse(memDC, prev->x - 3, prev->y - 3, prev->x + 3, prev->y + 3);
                        next = next->next;
                        prev = prev->next;
                    }
                    if(figure->complete == 1){
                        MoveToEx(memDC, figure->head->x, figure->head->y, NULL);
                        LineTo(memDC, prev->x, prev->y);
                        Ellipse(memDC, prev->x - 3, prev->y - 3, prev->x + 3, prev->y + 3);
                        if(figure->filled == 1){
                            RecurFill(memDC, figure->paintPoint.x, figure->paintPoint.y);
                        }
                    }else{
                        if(setup->currentNode != NULL){
                            if(setup->currentNode->allow == 1){
                                SelectObject(memDC, success);
                                SelectObject(memDC, pSuccess);
                            }else{
                                SelectObject(memDC, falue);
                                SelectObject(memDC, pFalue);
                            }
                            MoveToEx(memDC, prev->x, prev->y, NULL);
                            LineTo(memDC, setup->currentNode->x, setup->currentNode->y);
                            Ellipse(memDC, setup->currentNode->x - 3, setup->currentNode->y - 3, setup->currentNode->x + 3, setup->currentNode->y + 3);
                        }
                    }
                }else{
                    if(setup->currentNode != NULL){
                        if(setup->currentNode->allow == 1){
                            SelectObject(memDC, success);
                            SelectObject(memDC, pSuccess);
                        }else{
                            SelectObject(memDC, falue);
                            SelectObject(memDC, pFalue);
                        }
                        Ellipse(memDC, setup->currentNode->x - 3, setup->currentNode->y - 3, setup->currentNode->x + 3, setup->currentNode->y + 3);
                    }
                }
                figure = figure->next;
            }

            BitBlt(hdc, 0, 0, paintRect.right, paintRect.bottom, memDC, 0, 0, SRCCOPY);
            RestoreDC(memDC, saveDC);
            DeleteObject(bitMap);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
        }
        break;
        case WM_LBUTTONDOWN:
            if(setup->currentFigure == NULL){
                setup->currentFigure = AddFigure();
            }
            setup->currentNode = malloc(sizeof(struct Node));
            setup->currentNode->next = NULL;
            setup->currentNode->x = GET_X_LPARAM(lParam);
            setup->currentNode->y = GET_Y_LPARAM(lParam);
            if(setup->currentFigure->nodeNumber > 0){
                //check if line cross other figure
                struct Node* node = GetLastNode(setup->currentFigure);
                if(IsVectorIntersect(node, setup->currentNode) == TRUE)
                    setup->currentNode->allow = 0;
                else
                    setup->currentNode->allow = 1;
                //end check
            }else{
                if(IsPointInside(setup->currentNode->x, setup->currentNode->y) == TRUE)
                    setup->currentNode->allow = 0;
                else
                    setup->currentNode->allow = 1;

                //check if not in other figure
                //end check
            }
            RECT paintRect;
            GetClientRect(hwnd, &paintRect);
            InvalidateRect(hwnd, &paintRect, TRUE);
        break;
        case WM_LBUTTONUP:
            if(setup->currentNode != NULL && setup->currentFigure != NULL){
                if(setup->currentNode->allow == TRUE){
                    InsertNode(setup->currentFigure, setup->currentNode);
                    setup->currentNode = NULL;
                }else{
                    setup->currentNode = NULL;
                }
                RECT paintRect;
                GetClientRect(hwnd, &paintRect);
                InvalidateRect(hwnd, &paintRect, TRUE);
            }
        break;
        case WM_MOUSEMOVE:
            if(setup->currentNode != NULL){
                setup->currentNode->x = GET_X_LPARAM(lParam);
                setup->currentNode->y = GET_Y_LPARAM(lParam);
                //check if line cross other figure
                if(setup->currentFigure->nodeNumber > 0){
                    struct Node* node = GetLastNode(setup->currentFigure);
                    if(IsVectorIntersect(node, setup->currentNode) == 1)
                        setup->currentNode->allow = 0;
                    else
                        setup->currentNode->allow = 1;
                }else{
                    if(IsPointInside(setup->currentNode->x, setup->currentNode->y) == TRUE)
                        setup->currentNode->allow = 0;
                    else
                        setup->currentNode->allow = 1;
                }
                //end check
                RECT paintRect;
                GetClientRect(hwnd, &paintRect);
                InvalidateRect(hwnd, &paintRect, TRUE);
            }
        break;
        case WM_RBUTTONDOWN:
            if(setup->currentFigure != NULL){
                if(setup->currentFigure->nodeNumber > 2){
                    struct Node* node = GetLastNode(setup->currentFigure);
                    if(IsVectorIntersect(node, setup->currentFigure->head) == 0){
                        //check if not in other figure
                        setup->currentFigure->complete = 1;
                        setup->currentFigure = NULL;
                        RECT paintRect;
                        GetClientRect(hwnd, &paintRect);
                        InvalidateRect(hwnd, &paintRect, TRUE);
                    }       
                }
            }else{
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                if(IsPointInside(x, y) == TRUE){
                    if(currentPaintFigure != NULL){
                        currentPaintFigure->filled = 1;
                        currentPaintFigure->paintPoint.x = x;
                        currentPaintFigure->paintPoint.y = y;
                        RECT paintRect;
                        GetClientRect(hwnd, &paintRect);
                        InvalidateRect(hwnd, &paintRect, TRUE);
                    }
                }
            }
        break;
        case WM_CLOSE:
            DeleteObject(success);
            DeleteObject(falue);
            DeleteObject(solid);
            DeleteObject(pSuccess);
            DeleteObject(pFalue);
            DeleteObject(pSolid);
            DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

struct Figure* AddFigure(){
    if(headFigure == NULL){
        headFigure = malloc(sizeof(struct Figure));
        headFigure->head = NULL;
        headFigure->next = NULL;
        headFigure->nodeNumber = 0;
        headFigure->complete = 0;
        return headFigure;
    }else{
        struct Figure* node = headFigure;
        while(node->next != NULL){
            node = node->next;
        }
        node->next = malloc(sizeof(struct Figure));
        node = node->next;
        node->head = NULL;
        node->next = NULL;
        node->nodeNumber = 0;
        node->complete = 0;
        return node;
    }
}

struct Node* AddNode(struct Figure* figure){
    struct Node* head = figure->head;
    if(head == NULL){
        head = malloc(sizeof(struct Node));
        head->next = NULL;
        head->allow = 0;
        head->x = -1;
        head->y = -1;
        figure->nodeNumber++;
        return head;
    }else{
        struct Node* node = head;
        while(node->next != NULL){
            node = node->next;
        }
        node->next = malloc(sizeof(struct Node));
        node = node->next;
        node->next = NULL;
        node->allow = 0;
        node->x = -1;
        node->y = -1;
        figure->nodeNumber++;
        return node;
    }
}

void InsertNode(struct Figure* figure, struct Node* node){
    struct Node* newNode = GetLastNode(figure);
    if(newNode != NULL)
        newNode->next = node;
    else{
        figure->head = node;
    }
    figure->nodeNumber++;
}

struct Node* GetLastNode(struct Figure* figure){
    struct Node* head = figure->head;
    if(head == NULL){
        return NULL;
    }else{
        struct Node* node = head;
        while(node->next != NULL){
            node = node->next;
        }
        return node;
    }
}

WINBOOL Intersect(struct Node* a, struct Node* b, struct Node* c, struct Node* d){
    POINT pa, pb, pc, pd;
    pa.x = a->x;
    pa.y = a->y;
    pb.x = b->x;
    pb.y = b->y;
    pc.x = c->x;
    pc.y = c->y;
    pd.x = d->x;
    pd.y = d->y;

    if((pa.x == pd.x && pa.y == pd.y) || (pb.x == pc.x && pb.y == pc.y))
        return FALSE;
    
    POINT projection1, projection2, lenght;
    projection1.x = pb.x - pa.x;
    projection1.y = pb.y - pa.y;
    projection2.x = pd.x - pc.x;
    projection2.y = pd.y - pc.y;

    lenght.x = pa.x - pc.x;
    lenght.y = pa.y - pc.y;

    int denominator;
    int numeratorA;
    int numeratorB;

    denominator = projection2.y * projection1.x - projection2.x * projection1.y;
    numeratorA = projection2.x * lenght.y - projection2.y * lenght.x;
    numeratorB = projection1.x * lenght.y - projection1.y * lenght.x;

    if(denominator == 0 && (numeratorA != 0 || numeratorB != 0))
        return FALSE;

    
    float residual1 = (float)numeratorA / (float)denominator;
    float residual2 = (float)numeratorB / (float)denominator;

    if(residual1 < 0 || residual1 > 1)
        return FALSE;
    
    if(residual2 < 0 || residual2 > 1)
        return FALSE;

    return TRUE;
}

WINBOOL IsVectorIntersect(struct Node* a, struct Node* b){
    struct Figure* figure = headFigure;
    while(figure != NULL){
        if(figure->complete == 0){
            if(figure->nodeNumber > 1){
                struct Node* prev = figure->head;
                struct Node* next = prev->next;
                while(next != NULL){
                    if(Intersect(a, b, prev, next) == TRUE)
                        return TRUE;
                    next = next->next;
                    prev = prev->next;
                }
            }
        }else{
            struct Node* first = figure->head;
            struct Node* prev = figure->head;
            struct Node* next = prev->next;
            while(next != NULL){
                if(Intersect(a, b, prev, next) == TRUE)
                    return TRUE;
                next = next->next;
                prev = prev->next;
            }
            if(Intersect(a, b, first, prev) == TRUE)
                return TRUE;
        }
        figure = figure->next;
    }
    return FALSE;
}

WINBOOL IsPointInside(int x, int y){
    struct Node* vectorA = malloc(sizeof(struct Node));
    struct Node* vectorB = malloc(sizeof(struct Node));

    vectorA->x = x;
    vectorA->y = y;
    vectorB->x = x + 2000;
    vectorB->y = y + 10;

    struct Figure* figure = headFigure;

    int intersects = 0;

    while(figure  != NULL){
        if(figure->complete == 1){
            struct Node* first = figure->head;
            struct Node* prev = figure->head;
            struct Node* next = prev->next;
            while(next != NULL){
                if(Intersect(vectorA, vectorB, prev, next) == TRUE)
                    intersects++;
                next = next->next;
                prev = prev->next;
            }
            if(Intersect(vectorA, vectorB, first, prev) == TRUE)
                intersects++;
            if(intersects % 2 != 0){
                currentPaintFigure = figure;
                return TRUE;
            }
            intersects = 0;
        }
        figure = figure->next;
    }

    return FALSE;
}

void RecurFill(HDC hdc, int x, int y){
    COLORREF cross = RGB(0, 0, 0);
    RECT client;
    GetClientRect(mainHWND, &client);

    COLORREF up = GetPixel(hdc, x, y - 1);
    COLORREF down = GetPixel(hdc, x, y + 1);
    COLORREF left = GetPixel(hdc, x - 1, y);
    COLORREF right = GetPixel(hdc, x + 1, y);

    SetPixel(hdc, x, y, cross);

    if(up != cross)
        if(y > -1)
            RecurFill(hdc, x, y - 1);
    if(down != cross)
        if(y < client.bottom)
            RecurFill(hdc, x, y + 1);
    if(left != cross)
        if(x > -1)
            RecurFill(hdc, x - 1, y);
    if(right != cross)
        if(x < client.right)
            RecurFill(hdc, x + 1, y);
}