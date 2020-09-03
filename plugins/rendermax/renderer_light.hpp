#pragma once

#include <memory>
//#include <mutex>
#include <stack>
//#include <tuple>
//#include <unordered_map>
#include <hash_map>

#include "tinythread.h"

#include "renderer_opengl.hpp"
#include "Types.h"

// we are not using boost so let's cheat:
//template <class T>
//inline void hash_combine(std::size_t & seed, const T & v)
//{
//    std::hash<T> hasher;
//    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
//}

//namespace stdext
//{
//    template<typename S, typename T>
//    struct _Hash<std::pair<S, T>>
//    {
//        inline size_t operator()(const std::pair<S, T> & v) const
//        {
//            size_t seed = 0;
//            ::hash_combine(seed, v.first);
//            ::hash_combine(seed, v.second);
//            return seed;
//        }
//    };
//}

//namespace std
//{
//    template<typename S, typename T>
//    struct hash<pair<S, T>>
//    {
//        inline size_t operator()(const pair<S, T> & v) const
//        {
//            size_t seed = 0;
//            ::hash_combine(seed, v.first);
//            ::hash_combine(seed, v.second);
//            return seed;
//        }
//    };
//    template<typename S, typename T,typename V> struct hash<tuple<S, T, V>>
//    {
//        inline size_t operator()(const tuple<S, T, V> & v) const
//        {
//            size_t seed = 0;
//            ::hash_combine(seed,get<0>(v));
//            ::hash_combine(seed,get<1>(v));
//            ::hash_combine(seed,get<2>(v));
//            return seed;
//        }
//    };
//}
 //now we can hash pairs and tuples

#include "modules/MapCache.h"
bool isInRect(const df::coord2d& pos,const DFHack::rect2d& rect);
struct renderer_light : public renderer_wrap {
private:
    float light_adaptation;
    rgbf adapt_to_light(const rgbf& light)
    {
        const float influence=0.0001f;
        const float max_adapt=1;
        const float min_adapt=0;
        float intensity=(light.r+light.g+light.b)/3.0;
        light_adaptation=intensity*influence+light_adaptation*(1-influence);
        float delta=light_adaptation-intensity;

        rgbf ret;
        ret.r=light.r-delta;
        ret.g=light.g-delta;
        ret.b=light.b-delta;
        return ret;
        //if light_adaptation/intensity~1 then draw 1,1,1 (i.e. totally adapted)
        /*
            1. adapted -> 1,1,1 (full bright everything okay) delta=0 multiplier=?
            2. light adapted, real=dark -> darker delta>0   multiplier<1
            3. dark adapted, real=light -> lighter delta<0  multiplier>1
        */
        //if light_adaptation/intensity!=0 then draw

    }
    void colorizeTile(int x,int y)
    {
        const int tile = x*(df::global::gps->dimy) + y;
        old_opengl* p=reinterpret_cast<old_opengl*>(parent);
        float *fg = p->fg + tile * 4 * 6;
        float *bg = p->bg + tile * 4 * 6;
        float *tex = p->tex + tile * 2 * 6;
        rgbf light=lightGrid[tile];//for light adaptation: rgbf light=adapt_to_light(lightGrid[tile]);

        for (int i = 0; i < 6; i++) { //oh how sse would do wonders here, or shaders...
            *(fg++) *= light.r;
            *(fg++) *= light.g;
            *(fg++) *= light.b;
            *(fg++) = 1;

            *(bg++) *= light.r;
            *(bg++) *= light.g;
            *(bg++) *= light.b;
            *(bg++) = 1;
        }
    }
    void reinitLightGrid(int w,int h)
    {
        tthread::lock_guard<tthread::mutex> guard(dataMutex);
        lightGrid.resize(w*h,rgbf(1,1,1));
    }
    void reinitLightGrid()
    {
        reinitLightGrid(df::global::gps->dimy,df::global::gps->dimx);
    }

public:
    tthread::mutex dataMutex;
    std::vector12<rgbf> lightGrid;
    renderer_light(renderer* parent):renderer_wrap(parent),light_adaptation(1)
    {
        reinitLightGrid();
    }
    virtual void update_tile(int32_t x, int32_t y) {
        renderer_wrap::update_tile(x,y);
        tthread::lock_guard<tthread::mutex> guard(dataMutex);
        colorizeTile(x,y);
    };
    virtual void update_all() {
        renderer_wrap::update_all();
        tthread::lock_guard<tthread::mutex> guard(dataMutex);
        for (int x = 0; x < df::global::gps->dimx; x++)
            for (int y = 0; y < df::global::gps->dimy; y++)
                colorizeTile(x,y);
    };
    virtual void grid_resize(int32_t w, int32_t h) {
        renderer_wrap::grid_resize(w,h);
        reinitLightGrid(w,h);
    };
    virtual void resize(int32_t w, int32_t h) {
        renderer_wrap::resize(w,h);
        reinitLightGrid();
    }
    virtual void set_fullscreen()
    {
        renderer_wrap::set_fullscreen();
        reinitLightGrid();
    }
    virtual void zoom(df::zoom_commands z)
    {
        renderer_wrap::zoom(z);
        reinitLightGrid();
    }
};
class lightingEngine
{
public:
    lightingEngine(renderer_light* target):myRenderer(target){}
    virtual ~lightingEngine(){}
    virtual void reinit()=0;
    virtual void calculate()=0;

    virtual void updateWindow()=0;
    virtual void preRender()=0;

    virtual void loadSettings()=0;
    virtual void clear()=0;

    virtual void setHour(float h)=0;
    virtual void debug(bool enable)=0;
protected:
    renderer_light* myRenderer;
};
struct lightSource
{
    rgbf power;
    int radius;
    bool flicker;
    lightSource():power(0,0,0),radius(0),flicker(false)
    {

    }
    lightSource(rgbf power,int radius);
    float powerSquared()const
    {
        return power.r*power.r+power.g*power.g+power.b*power.b;
    }
    void combine(const lightSource& other);

};
struct matLightDef
{
    bool isTransparent;
    rgbf transparency;
    bool isEmiting;
    bool flicker;
    rgbf emitColor;
    int radius;
    matLightDef():isTransparent(false),transparency(0,0,0),isEmiting(false),emitColor(0,0,0),radius(0){}
    matLightDef(rgbf transparency,rgbf emit,int rad):isTransparent(true),transparency(transparency),
        isEmiting(true),emitColor(emit),radius(rad){}
    matLightDef(rgbf emit,int rad):isTransparent(false),transparency(0,0,0),isEmiting(true),emitColor(emit),radius(rad){}
    matLightDef(rgbf transparency):isTransparent(true),transparency(transparency),isEmiting(false){}
    lightSource makeSource(float size=1) const
    {
        if(size>0.999 && size<1.001)
            return lightSource(emitColor,radius);
        else
            return lightSource(emitColor*size,radius*size);//todo check if this is sane
    }
};
struct buildingLightDef
{
    matLightDef light;
    bool poweredOnly;
    bool useMaterial;
    float thickness;
    float size;
    buildingLightDef():poweredOnly(false),useMaterial(true),thickness(1.0f),size(1.0f){}
};
struct itemLightDef
{
    matLightDef light;
    bool haul;
    bool equiped;
    bool onGround;
    bool inBuilding;
    bool inContainer;
    bool useMaterial;
    itemLightDef():haul(true),equiped(true),onGround(true),inBuilding(false),inContainer(false),useMaterial(true){}
};
struct creatureLightDef
{
    matLightDef light;

};
class lightThread;
class lightingEngineViewscreen;
class lightThreadDispatch
{
    lightingEngineViewscreen *parent;
public:
    DFHack::rect2d viewPort;

    std::vector12<lightThread*> threadPool;
    std::vector12<lightSource>& lights;

    tthread::mutex occlusionMutex;
    tthread::condition_variable occlusionDone; //all threads wait for occlusion to finish
    bool occlusionReady;
    tthread::mutex unprocessedMutex;
    std::stack<DFHack::rect2d> unprocessed; //stack of parts of map where lighting is not finished
    std::vector12<rgbf>& occlusion;
    int& num_diffusion;

    tthread::mutex writeLock; //mutex for lightMap
    std::vector12<rgbf>& lightMap;

    tthread::condition_variable writesDone;
    int writeCount;

    lightThreadDispatch(lightingEngineViewscreen* p);
    ~lightThreadDispatch();
    void signalDoneOcclusion();
    void shutdown();
    void waitForWrites();

    int getW();
    int getH();
    void start(int count);
};
class lightThread
{
    std::vector12<rgbf> canvas;
    lightThreadDispatch& dispatch;
    DFHack::rect2d myRect;
    void work(); //main light calculation function
    void combine(); //combine existing canvas into global lightmap
public:
    tthread::thread *myThread;
    bool isDone; //no mutex, because bool is atomic
    lightThread(lightThreadDispatch& dispatch);
    ~lightThread();
    void run();
private:
    void doLight(int x,int y);
    void doRay(const rgbf& power,int cx,int cy,int tx,int ty,int num_diffuse);
    rgbf lightUpCell(rgbf power,int dx,int dy,int tx,int ty);
public:
    struct RayPlotter
    {
    public:
        explicit RayPlotter(lightThread* const _lt, const rgbf& _rf, int _x, int _y, int _num_diffuse) :
            lt(_lt), rf(_rf), x(_x), y(_y), num_diffuse(_num_diffuse) {}

        void operator()(int tx, int ty) const
        {
            lt->doRay(rf, x, y, tx, ty, num_diffuse);
        }

        lightThread* const lt;
        const rgbf& rf;
        int x;
        int y;
        int num_diffuse;
    };
    friend struct RayPlotter;
    struct CellLighter
    {
    public:
        explicit CellLighter(lightThread* const _lt) : lt(_lt) {}

        rgbf operator()(rgbf power, int dx, int dy, int tx, int ty) const
        {
            return lt->lightUpCell(power, dx, dy, tx, ty);
        }

        lightThread* const lt;
    };
    friend struct CellLighter;
};
class lightingEngineViewscreen:public lightingEngine
{
public:
    lightingEngineViewscreen(renderer_light* target);
    ~lightingEngineViewscreen();
    void reinit();
    void calculate();

    void updateWindow();
    void preRender();
    void loadSettings();
    void clear();

    void debug(bool enable){doDebug=enable;};
private:
    void fixAdvMode(int mode);
    df::coord2d worldToViewportCoord(const df::coord2d& in,const DFHack::rect2d& r,const df::coord2d& window2d) ;


    void doSun(const lightSource& sky,MapExtras::MapCache& map);
    void doOcupancyAndLights();
    rgbf propogateSun(MapExtras::Block* b, int x,int y,const rgbf& in,bool lastLevel);
    void doRay(std::vector12<rgbf> & target, rgbf power,int cx,int cy,int tx,int ty);
    void doFovs();
    void doLight(std::vector12<rgbf> & target, int index);
    rgbf lightUpCell(std::vector12<rgbf> & target, rgbf power,int dx,int dy,int tx,int ty);
    bool addLight(int tileId,const lightSource& light);
    void addOclusion(int tileId,const rgbf& c,float thickness);

    matLightDef* getMaterialDef(int matType,int matIndex);
    buildingLightDef* getBuildingDef(df::building* bld);
    creatureLightDef* getCreatureDef(df::unit* u);
    itemLightDef* getItemDef(df::item* it);

    //apply material to cell
    void applyMaterial(int tileId,const matLightDef& mat,float size=1, float thickness = 1);
    //try to find and apply material, if failed return false, and if def!=null then apply def.
    bool applyMaterial(int tileId,int matType,int matIndex,float size=1,float thickness = 1,const matLightDef* def=NULL);

    size_t inline getIndex(int x,int y)
    {
        return x*h+y;
    }
    df::coord2d inline getCoords(int index)
    {
        return df::coord2d(index/h, index%h);
    }
    //maps
    std::vector12<rgbf> lightMap;
    std::vector12<rgbf> ocupancy;
    std::vector12<lightSource> lights;

    //Threading stuff
    int num_diffuse; //under same lock as ocupancy
    lightThreadDispatch threading;
    //misc
    void setHour(float h){dayHour=h;};

    int getW()const {return w;}
    int getH()const {return h;}
public:
    void lightWorkerThread(void * arg);
private:
    rgbf getSkyColor(float v);
    bool doDebug;

    //settings
    float daySpeed;
    float dayHour; //<0 to cycle
    std::vector12<rgbf> dayColors; // a gradient of colors, first to 0, last to 24
    ///set up sane settings if setting file does not exist.
    void defaultSettings();

    static int parseMaterials(lua_State* L);
    static int parseSpecial(lua_State* L);
    static int parseBuildings(lua_State* L);
    static int parseItems(lua_State* L);
    static int parseCreatures(lua_State* L);
    //special stuff
    matLightDef matLava;
    matLightDef matIce;
    matLightDef matAmbience;
    matLightDef matCursor;
    matLightDef matWall;
    matLightDef matWater;
    matLightDef matCitizen;
    float levelDim;
    int adv_mode;
    ////materials
    //std::unordered_map<std::pair<int,int>,matLightDef> matDefs;
    ////buildings
    //std::unordered_map<std::tuple<int,int,int>,buildingLightDef> buildingDefs;
    ////creatures
    //std::unordered_map<std::pair<int,int>,creatureLightDef> creatureDefs;
    ////items
    //std::unordered_map<std::pair<int,int>,itemLightDef> itemDefs;

    struct BuildingDefKey
    {
    public:
        BuildingDefKey(int f, int s, int t) : first(f), second(s), third(t) {}
        BuildingDefKey() : first(0), second(0), third(0) {}

        int first;
        int second;
        int third;
    };
    struct BuildDefKeyHash
    {
        size_t operator()(const BuildingDefKey& k) const
        {
            return k.first*65537 + k.second*32769 + k.third;
        }
	    bool operator()(const BuildingDefKey& k1, const BuildingDefKey& k2) const
	    {
	        return k1.first == k2.first && k1.second == k2.second && k1.third == k2.third;
	    }
	    enum
	    {
	        bucket_size = 4,
	        min_buckets = 8
        };
    };
    struct DefPairHash
    {
        size_t operator()(const std::pair<int,int>& p) const
        {
            return p.first*65537 + p.second*32769;
        }
	    bool operator()(const std::pair<int,int>& p1, const std::pair<int,int>& p2) const
	    {
	        return p1.first == p2.first && p1.second == p2.second;
	    }
	    enum
	    {
	        bucket_size = 4,
	        min_buckets = 8
        };
    };
    typedef stdext::hash_map<std::pair<int,int>,matLightDef,DefPairHash> MatDefsMap;
    typedef stdext::hash_map<BuildingDefKey,buildingLightDef,BuildDefKeyHash> BuildingDefsMap;
    typedef stdext::hash_map<std::pair<int,int>,creatureLightDef,DefPairHash> CreatureDefsMap;
    typedef stdext::hash_map<std::pair<int,int>,itemLightDef,DefPairHash> ItemDefsMap;

    MatDefsMap matDefs;
    BuildingDefsMap buildingDefs;
    CreatureDefsMap creatureDefs;
    ItemDefsMap itemDefs;

    int w,h;
    DFHack::rect2d mapPort;
    friend class lightThreadDispatch;
};
rgbf blend(const rgbf& a,const rgbf& b);
rgbf blendMax(const rgbf& a,const rgbf& b);
