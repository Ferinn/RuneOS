#include <string>
#include "lvgl.h"
#include "ui.h"

enum Graphs {
    SENSOR = 0,
};

class GraphManager
{
    public:
        GraphManager();
        GraphManager(Graphs graphType);
        void NextData(double value);
    private:
        lv_obj_t* _graph;
        lv_chart_series_t* _graph_data;
};