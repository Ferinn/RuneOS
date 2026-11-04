#include "graphManager.hpp"

GraphManager::GraphManager()
{
    this->_graph = objects.sensor_graph;
}

GraphManager::GraphManager(Graphs graphType)
{
    switch (graphType)
    {
        case SENSOR:
            this->_graph = objects.sensor_graph;
            break;
    }

    _graph_data = lv_chart_add_series(_graph, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
}

void GraphManager::NextData(double value)
{
    lv_chart_set_next_value(_graph, _graph_data, value);
    return;
}