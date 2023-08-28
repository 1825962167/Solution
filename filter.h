#ifndef FILTER_H
#define FILTER_H
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/core.hpp>
#include<vector>
/*
 todo :
    Filter函数确认
 */

//定义任务类型
enum TaskType
{
    TASK_LEFTPARAL,
    TASK_RIGHTPARAL,
    TASK_LEFTADJUSTRANGE,
    TASK_RIGHTADJUSTRANGE,
    TASK_LEFTVISZERO,
    TASK_RIGHTVISZERO,
    TASK_GRADLEAN,              //Y
    TASK_IMAGEINCLINE,          //Y
    TASK_RELIMAGEINCLINE,       //Y
    TASK_AXISPARALDEGREE,       //Y
    TASK_LEFTRESOLUTION,
    TASK_RIGHTRESOLUTION,
    TASK_LEFTEXITPUPIL,
    TASK_RIGHTEXITPUPIL,
    TAKS_PUPILDISTANCERANGE,
    TASK_LEFTEXITPUPILDISTANCE,
    TASK_RIGHTEXITPUPILDISTANCE,
    TASK_LEFTRATE,
    TASK_RIGHTRATE,
    TASK_RATEDIF,
    TASK_LEFTFIELD,
    TASK_RIGHTFIELD,
    TASK_LEFTDISTORTION,
    TASK_RIGHTDISTORTION
};

struct Task
{
   TaskType type;
   std::vector<double> para;//参数
};

//抽象类
class Filter
{
public:
    //处理器函数要看后续的操作，还需要考虑下参数和输出
    virtual void filter(cv::Mat &mat1, cv::Mat &mat2)=0;
    virtual void  add(Task task) = 0;//添加任务
    virtual void clear() = 0;//清理任务
    static Filter* get();
    virtual ~Filter();
protected:
    Filter();
};

#endif // FILTER_H
