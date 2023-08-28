#include "filter.h"
#include"visionAlgorithm/imagepro.h"
#include<QMutex>
#include<QThread>

using namespace cv;
using namespace std;

class Cfilter:public Filter
{
public:
    std::vector<Task> tasks;
    QMutex mutex;
    void filter(cv::Mat &mat1, cv::Mat &mat2)
    {
        mutex.lock();
        ImagePro p;
//        p.Set(mat1, mat2);
//        for (int i = 0; i < tasks.size(); i++) {
//            switch (tasks[i].type) {
//            case TASK_MATCHTEMPLATE:
////                p.Gain(tasks[i].para[0], tasks[i].para[1]);
//                break;
//            default:
//                break;
//            }
//        }
        mutex.unlock();
//        return p.Get();
     }
    void  add(Task task) //添加任务
    {
        mutex.lock();
        tasks.push_back(task);
        mutex.unlock();
     }
    void clear() //清理任务
    {
        mutex.lock();
        tasks.clear();
        mutex.unlock();
     }
};

Filter::Filter()
{

}
Filter::~Filter()
{

}
Filter* Filter::get() {
    static Cfilter filter;//对抽象类实例化
    return &filter;
}
