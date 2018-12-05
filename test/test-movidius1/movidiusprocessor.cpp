#include "movidiusprocessor.h"



PyObject *convert(Mat frame)
{
    NDArrayConverter cvt;
    PyObject* obj;
    obj = cvt.toNDArray(frame);
    return obj;
}
