#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "util/log.h"
#include "util/time.h"

#define MAX_SAMPLES 600  // 10 seconds at 60 FPS
#define GRAPH_WIDTH 300
#define GRAPH_HEIGHT 100
#define GRAPH_X 300      // Left position of graph
#define GRAPH_Y 30      // Top position of graph
#define AXIS_COLOR D3DCOLOR_XRGB(128, 128, 128)  // Gray color for axes
#define GRAPH_COLOR D3DCOLOR_XRGB(255, 255, 0)   // Yellow color for the graph
#define TEXT_COLOR D3DCOLOR_XRGB(255, 255, 0)    // Yellow color for text

// // TODO have a separate module that wraps all of these somewhat
typedef HRESULT WINAPI (*func_D3DXCreateFontA)(
    struct IDirect3DDevice9 *device,
    INT height,
    UINT width,
    UINT weight,
    UINT miplevels,
    BOOL italic,
    DWORD charset,
    DWORD precision,
    DWORD quality,
    DWORD pitchandfamily,
    const char *facename,
    struct ID3DXFont **font);

typedef HRESULT WINAPI (*func_D3DXCreateLine)(
    struct IDirect3DDevice9 *device,
    struct ID3DXLine **line);

typedef struct {
    double frameTimes[MAX_SAMPLES];
    int currentIndex;
    double minTime;
    double maxTime;
    double avgTime;
    int sampleCount;
    
    ID3DXFont* font;        // For regular text
    ID3DXFont* smallFont;   // For axis labels
    ID3DXLine* line;
    bool resourcesInitialized;
} PerfMetrics;

PerfMetrics g_metrics = {0};

static HRESULT createLine(
    IDirect3DDevice9 *dev,
    struct ID3DXLine **line)
{
    HMODULE d3d9_24;

    d3d9_24 = GetModuleHandleA("d3dx9_24.dll");

    if (d3d9_24 == NULL) {
        log_fatal(
            "Failed to load d3dx9_24.dll to create a font for displaying "
            "framerate on monitor check.");
        return 0;
    }

    func_D3DXCreateLine d3dxCreateLine =
        (func_D3DXCreateLine) GetProcAddress(d3d9_24, "D3DXCreateLine");

    if (d3dxCreateLine == NULL) {
        log_fatal("Failed to find function D3DXCreateLine");
        return 0;
    }

    return d3dxCreateLine(
        dev,
        line);
}

static HRESULT createFontA(
    IDirect3DDevice9 *dev,
    INT height,
    UINT width,
    UINT weight,
    UINT miplevels,
    BOOL italic,
    DWORD charset,
    DWORD precision,
    DWORD quality,
    DWORD pitchandfamily,
    const char *facename,
    ID3DXFont **font)
{
    HMODULE d3d9_24;

    d3d9_24 = GetModuleHandleA("d3dx9_24.dll");

    if (d3d9_24 == NULL) {
        log_fatal(
            "Failed to load d3dx9_24.dll to create a font for displaying "
            "framerate on monitor check.");
        return 0;
    }

    func_D3DXCreateFontA d3dxCreateFontA =
        (func_D3DXCreateFontA) GetProcAddress(d3d9_24, "D3DXCreateFontA");

    if (d3dxCreateFontA == NULL) {
        log_fatal("Failed to find function D3DXCreateFontA");
        return 0;
    }

    return d3dxCreateFontA(
        dev,
        height,
        width,
        weight,
        miplevels,
        italic,
        charset,
        precision,
        quality,
        pitchandfamily,
        facename,
        font);
}

// Update performance metrics with new frame time
void UpdateMetrics(double frameTime) {
    g_metrics.frameTimes[g_metrics.currentIndex] = frameTime;
    g_metrics.currentIndex = (g_metrics.currentIndex + 1) % MAX_SAMPLES;
    
    if (g_metrics.sampleCount < MAX_SAMPLES)
        g_metrics.sampleCount++;
        
    // Update min/max/avg
    g_metrics.minTime = frameTime;
    g_metrics.maxTime = frameTime;
    g_metrics.avgTime = 0;
    
    for (int i = 0; i < g_metrics.sampleCount; i++) {
        double time = g_metrics.frameTimes[i];
        g_metrics.minTime = min(g_metrics.minTime, time);
        g_metrics.maxTime = max(g_metrics.maxTime, time);
        g_metrics.avgTime += time;
    }
    
    g_metrics.avgTime /= g_metrics.sampleCount;
}

bool InitializeOverlayResources(IDirect3DDevice9* device) {
    if (g_metrics.resourcesInitialized) return true;
    
    // Create main font (larger, for FPS display)
    if (FAILED(createFontA(device, 20, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        "Arial", &g_metrics.font))) {
        return false;
    }
    
    // Create smaller font for axis labels
    if (FAILED(createFontA(device, 12, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        "Arial", &g_metrics.smallFont))) {
        ID3DXFont_Release(g_metrics.font);
        return false;
    }
    
    if (FAILED(createLine(device, &g_metrics.line))) {
        ID3DXFont_Release(g_metrics.font);
        ID3DXFont_Release(g_metrics.smallFont);
        return false;
    }
    
    g_metrics.resourcesInitialized = true;
    return true;
}

void DrawGraph(IDirect3DDevice9* device) {
    // Draw the frame time graph
    D3DXVECTOR2 points[MAX_SAMPLES];
    float xStep = (float)GRAPH_WIDTH / (MAX_SAMPLES - 1);
    
    // Find min/max times in current sample range
    double minTime = g_metrics.frameTimes[0];
    double maxTime = g_metrics.frameTimes[0];
    for (int i = 0; i < g_metrics.sampleCount; i++) {
        int idx = (g_metrics.currentIndex - g_metrics.sampleCount + i + MAX_SAMPLES) % MAX_SAMPLES;
        double time = g_metrics.frameTimes[idx];
        minTime = min(minTime, time);
        maxTime = max(maxTime, time); 
    }
    
    // Add small padding to prevent graph from touching edges
    double padding = (maxTime - minTime) * 0.1;
    maxTime += padding;
    minTime = max(0, minTime - padding);
    
    float yScale = (float)GRAPH_HEIGHT / (maxTime - minTime);
    
    for (int i = 0; i < g_metrics.sampleCount; i++) {
        int idx = (g_metrics.currentIndex - g_metrics.sampleCount + i + MAX_SAMPLES) % MAX_SAMPLES;
        points[i].x = GRAPH_X + i * xStep;
        points[i].y = GRAPH_Y + GRAPH_HEIGHT - (g_metrics.frameTimes[idx] - minTime) * yScale;
    }
    
    ID3DXLine_SetWidth(g_metrics.line, 1.0f);
    ID3DXLine_Draw(g_metrics.line, points, g_metrics.sampleCount, GRAPH_COLOR);
}

// TODO add option to have fixed min and max for the Y scale (so that the graph doesn't scale when the frame rate changes)
// keep this an option though with uncapped to also see really bad frame times on screen
void DrawOverlay(IDirect3DDevice9* device) {
    if (!InitializeOverlayResources(device))
        return;
    
    // Draw black background
    D3DRECT rect = {GRAPH_X, GRAPH_Y - 30, GRAPH_X + GRAPH_WIDTH, GRAPH_Y + GRAPH_HEIGHT};
    IDirect3DDevice9_Clear(device, 1, &rect, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 0, 0, 0), 0, 0);
    
    // Draw X and Y axes
    D3DXVECTOR2 xAxis[] = {
        { .x = GRAPH_X, .y = GRAPH_Y + GRAPH_HEIGHT },
        { .x = GRAPH_X + GRAPH_WIDTH, .y = GRAPH_Y + GRAPH_HEIGHT }
    };
    
    D3DXVECTOR2 yAxis[] = {
        { .x = GRAPH_X, .y = GRAPH_Y },
        { .x = GRAPH_X, .y = GRAPH_Y + GRAPH_HEIGHT }
    };
    
    ID3DXLine_SetWidth(g_metrics.line, 1.0f);
    ID3DXLine_Draw(g_metrics.line, xAxis, 2, AXIS_COLOR);
    ID3DXLine_Draw(g_metrics.line, yAxis, 2, AXIS_COLOR);
    
    // Draw X axis labels (time)
    char buffer[32];
    RECT textRect;
    for (int i = 0; i <= 10; i++) {
        float x = GRAPH_X + (GRAPH_WIDTH * i / 10.0f);
        sprintf(buffer, "%.1fs", (10.0f - i));
        textRect.left = (LONG)x - 15;
        textRect.top = GRAPH_Y + GRAPH_HEIGHT + 5;
        textRect.right = (LONG)x + 15;
        textRect.bottom = GRAPH_Y + GRAPH_HEIGHT + 20;
        ID3DXFont_DrawTextA(g_metrics.smallFont, NULL, buffer, -1, &textRect, DT_CENTER, AXIS_COLOR);
    }

    // Find min/max times in current sample range
    double minTime = g_metrics.frameTimes[0];
    double maxTime = g_metrics.frameTimes[0];
    for (int i = 0; i < g_metrics.sampleCount; i++) {
        int idx = (g_metrics.currentIndex - g_metrics.sampleCount + i + MAX_SAMPLES) % MAX_SAMPLES;
        double time = g_metrics.frameTimes[idx];
        minTime = min(minTime, time);
        maxTime = max(maxTime, time); 
    }

    float yScale = (float)GRAPH_HEIGHT / (maxTime - minTime);
    
    // Draw Y axis labels (frame time)
    for (int i = 0; i <= 8; i++) {
        float y = GRAPH_Y + GRAPH_HEIGHT - (GRAPH_HEIGHT * i / 8.0f);
        float ms = minTime + ((maxTime - minTime) * i / 8.0f);
        sprintf(buffer, "%.1fms", ms);
        textRect.left = GRAPH_X - 45;
        textRect.top = (LONG)y - 6;
        textRect.right = GRAPH_X - 5;
        textRect.bottom = (LONG)y + 6;
        ID3DXFont_DrawTextA(g_metrics.smallFont, NULL, buffer, -1, &textRect, DT_RIGHT, AXIS_COLOR);
    }

    // TODO add some kind of coloring to the frame rate graph/line whenever the line is above or below a certain threshold of the reference line

    // Draw 60 FPS reference line (16.67ms)
    const float targetFrameTime = 1000.0f/60.0f; // 16.67ms
    float y60fps = GRAPH_Y + GRAPH_HEIGHT - ((targetFrameTime - minTime) * yScale);
    
    // Only draw reference line if it's within the visible range
    if (targetFrameTime >= minTime && targetFrameTime <= maxTime) {
        D3DXVECTOR2 refLine[2];
        refLine[0].x = GRAPH_X;
        refLine[0].y = y60fps;
        refLine[1].x = GRAPH_X + GRAPH_WIDTH;
        refLine[1].y = y60fps;
        
        ID3DXLine_SetWidth(g_metrics.line, 1.0f);
        ID3DXLine_Draw(g_metrics.line, refLine, 2, D3DCOLOR_ARGB(128, 255, 255, 0)); // Semi-transparent yellow

        // Draw reference line label
        char refBuffer[32];
        sprintf(refBuffer, "16.67ms (60 FPS)");
        RECT refTextRect;
        refTextRect.left = GRAPH_X + GRAPH_WIDTH + 5;
        refTextRect.top = y60fps - 8;  // Center text vertically with line
        refTextRect.right = refTextRect.left + 100;
        refTextRect.bottom = refTextRect.top + 16;
        ID3DXFont_DrawTextA(g_metrics.smallFont, NULL, refBuffer, -1, &refTextRect, DT_LEFT, D3DCOLOR_ARGB(128, 255, 255, 0));
    }

    // -------------------------------------------------------------------------

    // Draw the frame time graph
    D3DXVECTOR2 points[MAX_SAMPLES];
    float xStep = (float)GRAPH_WIDTH / (MAX_SAMPLES - 1);
    
    // Add small padding to prevent graph from touching edges
    double padding = (maxTime - minTime) * 0.1;
    maxTime += padding;
    minTime = max(0, minTime - padding);
    
    for (int i = 0; i < g_metrics.sampleCount; i++) {
        int idx = (g_metrics.currentIndex - g_metrics.sampleCount + i + MAX_SAMPLES) % MAX_SAMPLES;
        points[i].x = GRAPH_X + i * xStep;
        points[i].y = GRAPH_Y + GRAPH_HEIGHT - (g_metrics.frameTimes[idx] - minTime) * yScale;
    }
    
    ID3DXLine_SetWidth(g_metrics.line, 1.0f);
    ID3DXLine_Draw(g_metrics.line, points, g_metrics.sampleCount, GRAPH_COLOR);

    // -------------------------------------------------------------------------

    // Calculate current FPS
    double currentFrameTime = g_metrics.frameTimes[(g_metrics.currentIndex - 1 + MAX_SAMPLES) % MAX_SAMPLES];
    double fps = 1000.0 / currentFrameTime;
    
    // Draw FPS text
    sprintf(buffer, "%.3f FPS", fps);
    textRect.left = GRAPH_X + GRAPH_WIDTH - 100; 
    textRect.top = GRAPH_Y - 25;
    textRect.right = GRAPH_X + GRAPH_WIDTH;
    textRect.bottom = GRAPH_Y - 5;
    ID3DXFont_DrawTextA(g_metrics.font, NULL, buffer, -1, &textRect, DT_RIGHT, TEXT_COLOR);

    // Draw current frame time in ms
    sprintf(buffer, "%.3f ms", currentFrameTime);
    textRect.left = GRAPH_X + GRAPH_WIDTH - 200; // Position to the left of FPS
    textRect.top = GRAPH_Y - 25;
    textRect.right = GRAPH_X + GRAPH_WIDTH - 110; // Leave space for FPS text
    textRect.bottom = GRAPH_Y - 5;
    ID3DXFont_DrawTextA(g_metrics.font, NULL, buffer, -1, &textRect, DT_RIGHT, TEXT_COLOR);

    // TODO draw derivation graph and average derivation time over the current frame scope
    
    // Draw "Framerate" label
    textRect.left = GRAPH_X;
    textRect.top = GRAPH_Y - 25;
    textRect.right = GRAPH_X + 100;
    textRect.bottom = GRAPH_Y - 5;
    ID3DXFont_DrawTextA(g_metrics.font, NULL, "Framerate", -1, &textRect, DT_LEFT, TEXT_COLOR);
}

// Hooked Present function
void DrawFrameGraph(IDirect3DDevice9* device) {
    
    static uint64_t lastTime = 0;
    uint64_t currentTime = time_get_counter();
    
    if (lastTime != 0) {
        uint64_t frameTime = currentTime - lastTime;
        double frameTimeMs = time_get_elapsed_us(frameTime) / 1000.0;
        UpdateMetrics(frameTimeMs);
    }
    
    lastTime = currentTime;
    
    //IDirect3DDevice9_BeginScene(device);
    DrawOverlay(device);
    //IDirect3DDevice9_EndScene(device);
}