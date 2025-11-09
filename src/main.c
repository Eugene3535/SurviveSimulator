#include <stdlib.h>
#include <stdbool.h>

#include <Windows.h>

#include <glad/glad.h>
#include <wglext.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "camera.h"
#include "defines.h"

__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

GLboolean WGLExtensionSupported(const char* extension_name)
{
    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;
    _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC) wglGetProcAddress("wglGetExtensionsStringEXT");

    if (strstr(_wglGetExtensionsStringEXT(), extension_name) == NULL)
        return GL_FALSE;

    return GL_TRUE;
}

POINT screen_size;
float screen_ratio;

BOOL IsMouseBind = true;

// Map
typedef struct
{
    float x;
    float y;
    float z;
} Vertex;

typedef struct
{
    float u;
    float v;
} TexCoords;

typedef struct 
{
    float x;
    float y;
    float z;
    float scale;
    GLuint type;
} Object;

typedef struct
{
    Object* pObject;
    float dx;
    float dy;
    float dz;
    int count;
}Animation;

Animation anim;

typedef struct
{
    Object* tree_parts;
    GLuint part_count;
    GLuint type;
} CompositeTree;

Object* objects;
GLuint objectCount;

Vertex vertices[MAP_SIZE][MAP_SIZE];
TexCoords tex_coords[MAP_SIZE][MAP_SIZE];
Vertex normals[MAP_SIZE][MAP_SIZE];
GLuint indices[MAP_SIZE - 1][MAP_SIZE - 1][6];

GLuint indexCount = sizeof(indices) / sizeof(GLuint);

GLuint buffers[4];
// [0] vertex buffer object
// [1] tex coord buffer object
// [2] normal buffer object
// [3] index buffer object

Camera* pCamera;

GLuint tex_field;
GLuint tex_grass;
GLuint tex_red_flower;
GLuint tex_yellow_flower;
GLuint tex_mushroom;
GLuint tex_tree;
GLuint tex_tree2;
GLuint tex_wood;
GLuint tex_speed;
GLuint tex_eye;
GLuint tex_mortar;
GLuint tex_vision_potion;
GLuint tex_speed_potion;
GLuint tex_health_potion;

float plantVert[] = {-0.5, 0, 0,
                      0.5, 0, 0,
                      0.5, 0, 1,
                     -0.5, 0, 1,
                      0, -0.5, 0,
                      0, 0.5, 0,
                      0, 0.5, 1,
                      0, -0.5, 1
                      };

float plantUV [] = { 0,1, 1,1, 1,0, 0,0, 0,1, 1,1, 1,0, 0,0 };

int plantIndices[] = { 0,1,2, 2,3,0, 4,5,6, 6,7,4 };
int plantIndCount = sizeof(plantIndices) / sizeof(int);

float cube[] = 
{
    0, 0, 0,  1, 0, 0,  1, 1, 0,  0, 1, 0,
    0, 0, 1,  1, 0, 1,  1, 1, 1,  0, 1, 1,

    0, 0, 0,  1, 0, 0,  1, 1, 0,  0, 1, 0,
    0, 0, 1,  1, 0, 1,  1, 1, 1,  0, 1, 1
};

float cubeUV_tree_trunk[] =
{
    0.5, 0.5,  1, 0.5,    1, 0,    0.5, 0,
    0.5, 0.5,  1, 0.5,    1, 0,    0.5, 0,
    0,   0.5,  0.5, 0.5,  0, 0.5,  0.5, 0.5,
    0,   0,    0.5, 0,    0, 0,    0.5, 0
};

float cubeUV_tree_leafs[] =
{
    0, 1, 0.5, 1, 0.5, 0.5, 0, 0.5,
    0, 1, 0.5, 1, 0.5, 0.5, 0, 0.5,

    0, 0.5, 0.5, 0.5, 0, 0.5, 0.5, 0.5,
    0, 1,   0.5, 1,   0, 1,   0.5, 1
};

GLuint cubeIndices[] =
{
    0, 1, 2,  2, 3, 0,  4, 5, 6,  6, 7, 4,  8, 9, 13,  13, 12, 8,
    9, 10, 14,  14, 13, 9,  10, 11, 15,  15, 14, 10,  11, 8, 12,  12, 15, 11
};

GLuint cubeIndexCount = sizeof(cubeIndices) / sizeof(GLuint);
CompositeTree* trees;
GLuint tree_count;

float sunVertices[80];

typedef struct
{
    int object_num; 
    int color;
} SelectedObject;

SelectedObject selected_object_array[SELECTED_OBJECT_MAX];

bool IsSelectMode = false;
int selected_object_count;

typedef struct
{
    GLuint type;
    int x;
    int y;
    int width;
    int height;
} Slot;

Slot bag[BAG_SIZE];

float bagRectVert[] = { 0, 0, 1, 0, 1, 1, 0 ,1 };
float bagRectUV[] = { 0, 0, 1, 0, 1, 1, 0 ,1 };

int current_HP = 15;

float heartVert[] = 
{ 
    0.5f, 0.25f, 
    0.25f, 0.0f, 
    0.0f, 0.25f,
    0.5f, 1.0f,
    1.0f, 0.25f,
    0.75f, 0.0f
};

typedef struct
{
    int timer;
    int max_time;
} BuffTimer;

typedef struct 
{
    BuffTimer speed;
    BuffTimer eye;
} Buff;

Buff buffs;

GLuint handleItemType;
POINT mousePos;

typedef struct
{
    int x;
    int y;
    int width;
    int height;
    Slot items[3][3];
    Slot result_item;
    bool onDraw;
} CraftInterface;

CraftInterface craft_interface;

typedef struct
{
    GLuint components[3][3];
    GLuint result;
} Resipe;

Resipe* resipes;
const GLuint resipe_count = 3;


void CheckResipe()
{
    // for (const auto& resipe : resipes)
    for(GLuint i = 0; i < resipe_count; ++i)
    {
        Resipe* resipe = resipes + i;
        bool isResipe = true;

        for (GLuint i = 0; i < 3; ++i)
            for (GLuint j = 0; j < 3; ++j)       
                if (craft_interface.items[i][j].type != resipe->components[i][j])
                {
                    isResipe = false; 
                    continue;
                }

		if (isResipe)
		{
            for (GLuint i = 0; i < 3; ++i)
                for (GLuint j = 0; j < 3; ++j)
                    craft_interface.items[i][j].type = 0;

			craft_interface.result_item.type = resipe->result;

			break;
		}
	}
}


bool IsPointInSlot(const Slot* slot, int x, int y)
{
    return (x > slot->x && x < slot->x + slot->width &&
            y > slot->y && y < slot->y + slot->height);
}


void ResizeCraftInterface(int scale)
{
    craft_interface.width = scale * 6;
    craft_interface.height = scale * 4;
    craft_interface.x = (screen_size.x - craft_interface.width) * 0.5f;
    craft_interface.y = (screen_size.y - craft_interface.height) * 0.5f;

    int scale05 = (float)scale * 0.5f;

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
        {
            craft_interface.items[i][j].x = craft_interface.x + scale05 + i * scale;
            craft_interface.items[i][j].y = craft_interface.y + scale05 + j * scale;
            craft_interface.items[i][j].width = scale;
            craft_interface.items[i][j].height = scale;
        }

    craft_interface.result_item.x = craft_interface.x + scale05 + 4 * scale;
    craft_interface.result_item.y = craft_interface.y + scale05 + 1 * scale;
    craft_interface.result_item.width = scale;
    craft_interface.result_item.height = scale;
}


void ResizeWindow(int w, int h)
{
    if(glViewport)
    {
        glViewport(0, 0, w, h);
        screen_size.x = w;
        screen_size.y = h;
        screen_ratio = (float)w / (float)h;

        ResizeCraftInterface(50);
    }
}


void CollectObject(HWND hwnd);
float GetHeightInPoint(float x, float y);


void SetAnimation(Animation* pAnim, Object* pObj)
{
    if (pAnim->pObject) return;

    pAnim->pObject = pObj;
    pAnim->count = 10; // delay in ticks
    pAnim->dx = (pCamera->x - pObj->x) / (float)pAnim->count;
    pAnim->dy = (pCamera->y - pObj->y) / (float)pAnim->count;;
    pAnim->dz = ((pCamera->z - pObj->scale - 0.2f) - pObj->z) / (float)pAnim->count;;
}


void MoveAnimation(Animation* pAnim)
{
    if (pAnim->pObject)
    {
        pAnim->pObject->x += pAnim->dx;
        pAnim->pObject->y += pAnim->dy;
        pAnim->pObject->z += pAnim->dz;

        if (--pAnim->count < 1)
        {
            for (int i = 0; i < BAG_SIZE; ++i)
            {
                if (bag[i].type == 0) 
                {
                    bag[i].type = pAnim->pObject->type;
                    break;
                }

                if (i < BAG_SIZE)
                {
                    pAnim->pObject->x = rand() % MAP_SIZE;
                    pAnim->pObject->y = rand() % MAP_SIZE;
                }
            }
            pAnim->pObject->z = GetHeightInPoint(pAnim->pObject->x, pAnim->pObject->y);
            pAnim->pObject = NULL;
        }
    }
}


GLuint GetTexture(const char* filepath)
{
    int w, h, c; // width, height, channels
    GLuint handle = 0;
    unsigned char* pData = stbi_load(filepath, &w, &h, &c, STBI_rgb_alpha);

    if ( ! pData)
        return handle;

    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(pData);

    return handle;
}


void CalcNormals(const Vertex* a, const Vertex* b, const Vertex* c, Vertex* n)
{
    float wrki = 0;
    Vertex v1 = { a->x - b->x, a->y - b->y, a->z - b->z };
    Vertex v2 = { b->x - c->x, b->y - c->y, b->z - c->z };

    n->x = (v1.y * v2.z - v1.z *  v2.y);
    n->y = (v1.z * v2.x - v1.x *  v2.z);
    n->z = (v1.x * v2.y - v1.y *  v2.x);

    wrki = sqrt(n->x * n->x + n->y * n->y + n->z * n->z);

    n->x /= wrki;
    n->y /= wrki;
    n->z /= wrki;
}


bool IsInBounds(float x, float y)
{
    return (x >= 0) && (x < MAP_SIZE) && (y >= 0) && (y < MAP_SIZE);
}


void CreateHill(int x, int y, int r, int h)
{
    for(int i = x - r; i < x + r; ++i)
        for(int j = y - r; j < y + r; ++j)
    {
        if(IsInBounds(i, j))
        {
            float L = sqrt((x - i) * (x - i) + (y - j) * (y - j));

            if(L < r)
            {
                L = L / r * GLM_PI_2;
                vertices[i][j].z += cos(L) * h;
            }
        }
    }
}


void CreateCompositeTree(CompositeTree* tree, GLuint type, float x, float y)
{
    tree->type = type;
    float z = GetHeightInPoint(x + 0.5f, y + 0.5f) - 0.5f; // tree`s trunk center height
    GLuint tree_height = 6;
    GLuint leafs = 5 * 5 * 2 - 2 + 3 * 3 * 2;

    tree->part_count = tree_height + leafs;
    tree->tree_parts = (Object*)malloc(tree->part_count * sizeof(Object));

    for (GLuint i = 0; i < tree->part_count; ++i)  
    {
        Object* part = tree->tree_parts + i;
        part->scale = 1;
        part->type = 1;
        part->x = x;
        part->y = y;
        part->z = z + i;
    }

    GLuint pos = tree_height;

    for (int k = 0; k < 2; ++k)
        for (int i = x - 2; i <= x + 2; ++i)
            for (int j = y - 2; j <= y + 2; ++j)
            {
                if ((i != x) || (j != y))
                {
                    tree->tree_parts[pos].type = 2;
                    tree->tree_parts[pos].x = i;
                    tree->tree_parts[pos].y = j;
                    tree->tree_parts[pos].z = z + tree_height - 2 + k;
                    pos++;
                }
            }

    for (int k = 0; k < 2; ++k)
        for (int i = x - 1; i <= x + 1; ++i)
            for (int j = y - 1; j <= y + 1; ++j)
            {
                 tree->tree_parts[pos].type = 2;
                 tree->tree_parts[pos].x = i;
                 tree->tree_parts[pos].y = j;
                 tree->tree_parts[pos].z = z + tree_height + k;
                 pos++;
            }
}


float GetHeightInPoint(float x, float y)
{
    if(IsInBounds(x, y))
    {
        int cX = (int)x;
        int cY = (int)y;

        float h1 = ((1 - (x - cX)) * vertices[cX][cY].z + (x - cX) * vertices[cX + 1][cY].z);
        float h2 = ((1 - (x - cX)) * vertices[cX][cY + 1].z + (x - cX) * vertices[cX + 1][cY + 1].z);

        return (1 - (y - cY)) * h1 + (y - cY) * h2;
    }

    return 0;
}


void InitMap()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.99);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float l = 0.5f;
    float a = GLM_PI * 2 / 40;

    for (int i = 0; i < 40; i += 2)
    {
        sunVertices[i] = sin(a * i) * l;
        sunVertices[i + 1] = cos(a * i) * l;
    }

    tex_field = GetTexture("res/textures/field.png");
    tex_grass = GetTexture("res/textures/grass.png");
    tex_red_flower = GetTexture("res/textures/flower.png");
    tex_yellow_flower = GetTexture("res/textures/flower2.png");
    tex_mushroom = GetTexture("res/textures/mushroom.png");
    tex_tree = GetTexture("res/textures/tree.png");
    tex_tree2 = GetTexture("res/textures/tree2.png");
    tex_wood = GetTexture("res/textures/wood.png");
    tex_speed = GetTexture("res/textures/speed_icon.png");
    tex_eye = GetTexture("res/textures/eye_icon.png");
    tex_mortar = GetTexture("res/textures/mortar.png");
    tex_vision_potion = GetTexture("res/textures/potion_eye.png");
    tex_speed_potion = GetTexture("res/textures/potion_speed.png");
    tex_health_potion = GetTexture("res/textures/potion_life.png");

    bag[0].type = tex_mortar;

    GLuint eye_resipe[3][3] = 
    {
        tex_yellow_flower, 0, tex_yellow_flower,
        0, tex_mushroom, 0,
        tex_yellow_flower, 0, tex_yellow_flower
    };

    GLuint speed_resipe[3][3] = 
    {
        tex_red_flower, tex_red_flower, tex_red_flower,
        0, 0, 0,
        tex_red_flower, tex_red_flower, tex_red_flower
    };

    GLuint health_resipe[3][3] = 
    {
        0, tex_mushroom, 0,
        tex_mushroom, tex_mushroom, tex_mushroom,
        0, tex_mushroom, 0
    };

    resipes = (Resipe*)calloc(3, sizeof(Resipe));
    Resipe* resipe = resipes;

    for (GLuint i = 0; i < 3; ++i)
        for (GLuint j = 0; j < 3; ++j)
            resipe->components[i][j] = eye_resipe[i][j];

    resipe->result = tex_vision_potion;

    ++resipe;

    for (GLuint i = 0; i < 3; ++i)
        for (GLuint j = 0; j < 3; ++j)
            resipe->components[i][j] = speed_resipe[i][j];

    resipe->result = tex_speed_potion;

    ++resipe;

    for (GLuint i = 0; i < 3; ++i)
        for (GLuint j = 0; j < 3; ++j)
            resipe->components[i][j] = health_resipe[i][j];

    resipe->result = tex_health_potion;

    for(int i = 0; i < MAP_SIZE; ++i)
        for(int j = 0; j < MAP_SIZE; ++j)
        {
            float dc = rand() % 20 * 0.01f;

            vertices[i][j].x = i;
            vertices[i][j].y = j;
            vertices[i][j].z = (rand() % 10) * 0.02f;

            tex_coords[i][j].u = i;
            tex_coords[i][j].v = j;
        }

    for(int i = 0; i < MAP_SIZE - 1; ++i)
    {
        int pos = i * MAP_SIZE;

        for(int j = 0; j < MAP_SIZE - 1; ++j)
        {
            indices[i][j][0] = pos;
            indices[i][j][1] = pos + 1;
            indices[i][j][2] = pos + 1 + MAP_SIZE;

            indices[i][j][3] = pos + 1 + MAP_SIZE;
            indices[i][j][4] = pos + MAP_SIZE;
            indices[i][j][5] = pos;

            pos++;
        }
    }

    for(int i = 0; i < 10; ++i)
        CreateHill(rand() % MAP_SIZE, rand() % MAP_SIZE, rand() % 50, rand() % 10);

    for(int i = 0; i < MAP_SIZE - 1; ++i)
        for(int j = 0; j < MAP_SIZE - 1; ++j)
           CalcNormals(&vertices[i][j], &vertices[i + 1][j], &vertices[i][j + 1], &normals[i][j]);

//  VBO
    glGenBuffers(4, buffers);

//  Vertices
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

//  Texture coords
    glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tex_coords), tex_coords, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

//  Normals
    glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

//  Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLuint grassCount = 2000;
    GLuint mushroomCount = 30;
    GLuint treeCount = 40;
    objectCount = grassCount + mushroomCount + treeCount;

    objects = (Object*)malloc(objectCount * sizeof(Object));

    for(int i = 0; i < objectCount; ++i)
    {
        if(i < grassCount)
        {
            objects[i].type = rand() % 10 != 0 ? tex_grass : (rand() % 2 != 0 ? tex_red_flower : tex_yellow_flower);
            objects[i].scale = 0.7f + (rand() % 5) * 0.1f;
        }
        else if(i < grassCount + mushroomCount)
        {
            objects[i].type = tex_mushroom;
            objects[i].scale = 0.2f + (rand() % 10) * 0.01f;
        }
        else
        {
            objects[i].type = rand() % 2 == 0 ? tex_tree : tex_tree2;
            objects[i].scale = 4 + (rand() % 14);
        }

        objects[i].x = rand() % MAP_SIZE;
        objects[i].y = rand() % MAP_SIZE;
        objects[i].z = GetHeightInPoint(objects[i].x, objects[i].y);
    }

    tree_count = 50;
    trees = (CompositeTree*)malloc(tree_count * sizeof(CompositeTree));

    for (GLuint i = 0; i < tree_count; ++i)
        CreateCompositeTree(trees + i, tex_wood, rand() % MAP_SIZE, rand() % MAP_SIZE);
}


void MovePlayer()
{
    int forward = GetKeyState('W') < 0 ? 1 : (GetKeyState('S') < 0 ? - 1 : 0);
    int right = GetKeyState('D') < 0 ? 1 : (GetKeyState('A') < 0 ? - 1 : 0);
    float speed = 0.1f + (buffs.speed.timer > 0 ? 0.2f : 0.0f);
    
    moveCamera(pCamera, forward, right, speed);

    if(IsMouseBind)
        rotateByMouseCamera(pCamera, 400, 400, 0.2f);

    pCamera->z = GetHeightInPoint(pCamera->x, pCamera->y) + 1.7f;
}


void UpdateBuffTimer(BuffTimer* bt)
{
    if (--bt->timer > 0)
    {
        if (bt->timer <= 0)
            bt->max_time = 0;
    }
}


void UpdatePlayerState()
{
    static int hungry = 0;

    if (++hungry > 200)
    {
        hungry = 0;
        --current_HP;

        if (current_HP < 1)
            PostQuitMessage(0);
    }

    UpdateBuffTimer(&buffs.speed);
    UpdateBuffTimer(&buffs.eye);
}


void DrawScene()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glFrustum(-screen_ratio * 0.1, screen_ratio * 0.1, -0.1, 0.1, 0.1 * 2, 1000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_DEPTH_TEST);

    static float alpha = 0;
    alpha += 0.03f;

    if (alpha > 180.0f) alpha -= 360.0f;

    float kcc = 1 - (fabs(alpha) / 180.0f);

    float sunset = 40.0f;
    float k = 90 - abs(alpha);
    k = (sunset - abs(k));
    k = k < 0 ? 0 : k / sunset;

    if(!IsSelectMode)
        glClearColor(0.6f * kcc, 0.8f * kcc, 1.0f * kcc, 0.0f);
    else
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!IsSelectMode)
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
    }
    else
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
    }

    MoveAnimation(&anim);

    glPushMatrix();

//  Draw sun
    if (!IsSelectMode)
    {
        glPushMatrix();
        {
            glRotatef(-pCamera->angleX, 1, 0, 0);
            glRotatef(-pCamera->angleZ, 0, 0, 1);
            glRotatef(alpha, 0, 1, 0);
            glTranslatef(0, 0, 20);
            glDisable(GL_DEPTH_TEST);

            glDisable(GL_TEXTURE_2D);
            glColor3f(1, 1 - k * 0.8f, 1 - k);

            glEnableClientState(GL_VERTEX_ARRAY);

            glVertexPointer(2, GL_FLOAT, 0, sunVertices);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 40);

            glDisableClientState(GL_VERTEX_ARRAY);

            glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);
        }
        glPopMatrix();
    }

    applyCamera(pCamera);

    glPushMatrix();
    {
        glRotatef(alpha, 0, 1, 0);

        float pos[] = { 0.0f, 0.0f, 1.0f, 0.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, pos); 

        float mas[] = { 1 + k * 2, 1, 1, 0 };
        glLightfv(GL_LIGHT0, GL_DIFFUSE, mas);

        float clr = kcc * 0.15f + 0.05f;
        float mas0[] = { clr, clr, clr, 0 };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, mas0);
    }
    glPopMatrix();

//  Draw terrain
    if (!IsSelectMode)
    {
        glBindTexture(GL_TEXTURE_2D, tex_field);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
        glVertexPointer(3, GL_FLOAT, 0, NULL);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
        glTexCoordPointer(2, GL_FLOAT, 0, NULL);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
        glNormalPointer(GL_FLOAT, 0, NULL);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[3]);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, NULL);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

//  Draw objects
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, plantVert);
    glTexCoordPointer(2, GL_FLOAT, 0, plantUV);
    glNormal3f(0, 0, 1);

    selected_object_count = 0;
    GLubyte selected_color = 1;

    for(int i = 0; i < objectCount; ++i)
    {
        if (IsSelectMode)
        {
            if (objects[i].type == tex_tree || objects[i].type == tex_tree2)
                continue;

            static const int radius = 3;

            if (objects[i].x > pCamera->x - radius &&
                objects[i].x < pCamera->x + radius &&
                objects[i].y > pCamera->y - radius &&
                objects[i].y < pCamera->y + radius)
            {
                glColor3ub(selected_color, 0, 0);
                selected_object_array[selected_object_count].color = selected_color++;
                selected_object_array[selected_object_count++].object_num = i;

                if (selected_color >= 255) break;
            }
            else 
                continue;
        }
        else
        {
            if (objects[i].type == tex_mushroom && buffs.eye.timer > 0)
                glDisable(GL_LIGHTING);
        }

        glBindTexture(GL_TEXTURE_2D, objects[i].type);
        glPushMatrix();

        glTranslatef(objects[i].x, objects[i].y, objects[i].z);
        glScalef(objects[i].scale, objects[i].scale, objects[i].scale);

        glDrawElements(GL_TRIANGLES, plantIndCount, GL_UNSIGNED_INT, plantIndices);

        glPopMatrix();

        if(!IsSelectMode)
            if (objects[i].type == tex_mushroom && buffs.eye.timer > 0)
                glEnable(GL_LIGHTING);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//  Draw composite trees
    if (!IsSelectMode)
    {
        for(GLuint n = 0; n < tree_count; ++n)
        {
            CompositeTree* tree = trees + n;

            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            glVertexPointer(3, GL_FLOAT, 0, cube);
            glNormal3f(0, 0, 1);
            glBindTexture(GL_TEXTURE_2D, tree->type);

            for (GLuint i = 0; i < tree->part_count; ++i)     
            {
                Object* part = tree->tree_parts + i;

                if (part->type == 1)
                    glTexCoordPointer(2, GL_FLOAT, 0, cubeUV_tree_trunk);
                else
                    glTexCoordPointer(2, GL_FLOAT, 0, cubeUV_tree_leafs);

                glPushMatrix();

                glTranslatef(part->x, part->y, part->z);
                glScalef(part->scale, part->scale, part->scale);

                glDrawElements(GL_TRIANGLES, cubeIndexCount, GL_UNSIGNED_INT, cubeIndices);

                glPopMatrix();

            }

            glBindTexture(GL_TEXTURE_2D, 0);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisableClientState(GL_VERTEX_ARRAY);
        }
    }

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}


void CollectObject(HWND hwnd)
{
    IsSelectMode = true;
    DrawScene();
    IsSelectMode = false;

    RECT rect;
    GetClientRect(hwnd, &rect);

    GLubyte clr[3];
    glReadPixels((float)rect.right * 0.5f, (float)rect.bottom * 0.5f, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, clr);

    if (clr[0] > 0)
    {
        for (int i = 0; i < selected_object_count; ++i) 
        {
            if (selected_object_array[i].color == clr[0])
            {
                //objects[selected_object_array[i].object_num].z = -1000;
                SetAnimation(&anim, objects + selected_object_array[i].object_num);
            }
        }
    }
}


void DrawCell(int x, int y, int scaleX, int scaleY, GLuint type)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, bagRectVert);
    glTexCoordPointer(2, GL_FLOAT, 0, bagRectUV);

    glPushMatrix();
    {
        glTranslatef(x, y, 0);
        glScalef(scaleX, scaleY, 1);
        glColor3ub(110, 95, 73);
        glDisable(GL_TEXTURE_2D);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        if (type)
        {
            glColor3f(1, 1, 1);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, type);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }

        glColor3ub(160, 146, 116);
        glLineWidth(3);
        glDisable(GL_TEXTURE_2D);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
    }
    glPopMatrix();

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}


void DrawBag(int x, int y, int scale)
{
    for (int i = 0; i < BAG_SIZE; ++i)
        DrawCell(x + i * scale, y, scale, scale, bag[i].type);
}


void DrawItemInHand()
{
    if(handleItemType > 0 && ! IsMouseBind)
        DrawCell(mousePos.x, mousePos.y, 50, 50, handleItemType);
}


GLuint GetItemCount(GLuint type)
{
    GLuint cnt = 0;

    for (int i = 0; i < BAG_SIZE; ++i)
        if (bag[i].type == type) cnt++;
    
    return cnt;
}


void ClickOnBag(int x, int y, int scale, int mx, int my, UINT button)
{
    if (my < y || my > y + scale) return;

    for (int i = 0; i < BAG_SIZE; ++i)
    {
        if (mx > x + i * scale && mx < x + (i + 1) * scale)
        {
            if (button == WM_LBUTTONDOWN)
            {
                GLuint type = handleItemType;
                handleItemType = bag[i].type;
                bag[i].type = type;
            }
            else if (bag[i].type == tex_mortar)
            {
                craft_interface.onDraw = !craft_interface.onDraw;
            }
            else if (bag[i].type == tex_mushroom)
            {
                ++current_HP;

                if (current_HP > HEALTH_MAX) 
                    current_HP = HEALTH_MAX;

                bag[i].type = 0;
            }
            else if (bag[i].type == tex_health_potion)
            {
                current_HP += 15;

                if (current_HP > HEALTH_MAX)
                    current_HP = HEALTH_MAX;

                bag[i].type = 0;
            }
            else if (bag[i].type == tex_speed_potion)
            {
                buffs.speed.timer = 3600;
                buffs.speed.max_time = 3600;
                bag[i].type = 0;
            }
            else if (bag[i].type == tex_vision_potion)
            {
                buffs.eye.timer = 3600;
                buffs.eye.max_time = 3600;
                bag[i].type = 0;
            }
            else
                bag[i].type = 0;
        }
    }
}


void DrawHealthLine(int x, int y, int scale)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, heartVert);

    for (int i = 0; i < HEALTH_MAX; ++i)
    {
        glPushMatrix();

        glTranslatef(x + i * scale, y, 0);
        glScalef(scale, scale, 1);
        glColor3f((i < current_HP) ? 1 : 0, 0, 0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

        glPopMatrix();
    }

    glDisableClientState(GL_VERTEX_ARRAY);
}


void DrawCross()
{
    static const float crossVert[] = { 0, -1, 0, 1, -1, 0, 1, 0 };

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, crossVert);

    glColor3f(1, 1, 1);

    glPushMatrix();
    {
        glTranslatef((float)screen_size.x * 0.5f, (float)screen_size.y * 0.5f, 0);
        glScalef(15, 15, 1);
        glLineWidth(3);

        glDrawArrays(GL_LINES, 0, 4);
    }
    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);
}


void DrawBuff(int x, int y, int scale, const BuffTimer* buff, GLuint tex_id)
{
    if (buff->timer > 0)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(2, GL_FLOAT, 0, bagRectVert);
        glTexCoordPointer(2, GL_FLOAT, 0, bagRectUV);

        glPushMatrix();
        {
            glTranslatef(x, y, 0);
            glScalef(scale, scale, 1);
            glColor3f(1, 1, 1);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, tex_id);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
            glBindTexture(GL_TEXTURE_2D, 0);

            glScalef(1, 1 - ((float)buff->timer / (float)buff->max_time), 1);
            glColor4f(1, 1, 1, 0.5f);
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_TEXTURE_2D);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);           
            glEnable(GL_ALPHA_TEST);
        }
        glPopMatrix();

        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
}


void ClickOnCraftInterfase(int mx, int my, int button)
{
    if (!craft_interface.onDraw || button != WM_LBUTTONDOWN) return;

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
        {
            if (IsPointInSlot(&craft_interface.items[i][j], mx, my))
            {
                GLuint type = handleItemType;
                handleItemType = craft_interface.items[i][j].type;
                craft_interface.items[i][j].type = type;
            }
        }

    if (IsPointInSlot(&craft_interface.result_item, mx, my) && handleItemType == 0 && craft_interface.result_item.type > 0)
    {
        handleItemType = craft_interface.result_item.type;
        craft_interface.result_item.type = 0;

        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                craft_interface.result_item.type = 0;
    }

    CheckResipe();
}


void DrawCraftInterface()
{
    if (!craft_interface.onDraw || IsMouseBind) return;

    DrawCell(craft_interface.x, craft_interface.y, craft_interface.width, craft_interface.height, 0);

    for (int i = 0; i < 3; ++i) 
        for (int j = 0; j < 3; ++j)
            DrawCell(
                craft_interface.items[i][j].x,
                craft_interface.items[i][j].y,
                craft_interface.items[i][j].width,
                craft_interface.items[i][j].height, 
                craft_interface.items[i][j].type);

    DrawCell(
        craft_interface.result_item.x,
        craft_interface.result_item.y,
        craft_interface.result_item.width,
        craft_interface.result_item.height,
        craft_interface.result_item.type);
}


void DrawInterface()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, screen_size.x, screen_size.y, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    DrawBag(10, 10, 50);
    DrawHealthLine(10, 70, 30);
    DrawCross();
    DrawCraftInterface();
    DrawBuff(10, 110, 50, &buffs.speed, tex_speed);
    DrawBuff(60, 110, 50, &buffs.eye, tex_eye);
    DrawItemInHand();
}


void ReleaseResources()
{
    glDeleteTextures(1, &tex_field);
    glDeleteTextures(1, &tex_grass);
    glDeleteTextures(1, &tex_red_flower);
    glDeleteTextures(1, &tex_yellow_flower);
    glDeleteTextures(1, &tex_mushroom);
    glDeleteTextures(1, &tex_tree);
    glDeleteTextures(1, &tex_tree2);
    glDeleteTextures(1, &tex_tree2);
    glDeleteTextures(1, &tex_speed);
    glDeleteTextures(1, &tex_eye);
    glDeleteTextures(1, &tex_mortar);
    glDeleteTextures(1, &tex_vision_potion);
    glDeleteTextures(1, &tex_speed_potion);
    glDeleteTextures(1, &tex_health_potion);

    glDeleteBuffers(4, buffers);

    for (GLuint i = 0; i < tree_count; ++i)
        free(trees[i].tree_parts);

    free(trees);
    free(objects);
}


int main()
{
    WNDCLASSA wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;

    /* register window class */
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";

    if (!RegisterClassA(&wcex))
        return -1;

    hwnd = CreateWindowEx(0, "GLSample", "OpenGL Sample", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1200, 900, NULL, NULL, NULL, NULL);
    ShowWindow(hwnd, SW_SHOWNORMAL);
    SetCursor(wcex.hCursor);

    EnableOpenGL(hwnd, &hDC, &hRC);

    if(!gladLoadGL())
        return -1;

    PFNWGLSWAPINTERVALEXTPROC    wglSwapIntervalEXT = NULL;
    PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = NULL;

    if(WGLExtensionSupported("WGL_EXT_swap_control"))
    {
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
        wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC) wglGetProcAddress("wglGetSwapIntervalEXT");
        wglSwapIntervalEXT(1);
    }

    glEnable(GL_DEPTH_TEST);

    Camera camera;
    camera.x = 0;
    camera.y = 0;
    camera.z = 1.7;
    camera.angleX = 90;
    camera.angleZ = 0;
    pCamera = &camera;

    RECT rect;
    GetClientRect(hwnd, &rect);
    ResizeWindow(rect.right, rect.bottom);

    InitMap();

    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */
            GetCursorPos(&mousePos);
            ScreenToClient(hwnd, &mousePos);

            if(GetForegroundWindow() == hwnd)
                MovePlayer();

            UpdatePlayerState();
            DrawScene();
            DrawInterface();

            SwapBuffers(hDC);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    ReleaseResources();

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        if (IsMouseBind)
            CollectObject(hwnd);
        else
            ClickOnBag(10, 10, 50, LOWORD(lParam), HIWORD(lParam), uMsg);
            ClickOnCraftInterfase(LOWORD(lParam), HIWORD(lParam), uMsg);
        break;

    case WM_SIZE:
        ResizeWindow(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_SETCURSOR:
        ShowCursor(!IsMouseBind);
        break;

    case WM_DESTROY:
        return 0;

    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;

        case 'E':
            IsMouseBind = !IsMouseBind;
            SetCursorPos(400, 400);
            if (IsMouseBind)
                while (ShowCursor(FALSE) >= 0);
            else
                while (ShowCursor(TRUE) <= 0);
            break;
        }
    }
    break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}
