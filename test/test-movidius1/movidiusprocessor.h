#ifndef MOVIDIUSPROCESSOR_H
#define MOVIDIUSPROCESSOR_H

#include <Python.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <mutex>
#include "conversion.h"
using namespace cv;
using namespace std;


template <typename T>
class py_arg_t{
public:
    py_arg_t(T d,string n):data(d),des(n)
    {}
    T data;
    string des;
};
template <typename... T>
void set_item(PyObject* pArgs,int index)
{
}
template <class... T,typename HEAD>
void set_item(PyObject* pArgs,int index,HEAD arg,T...args)
{
    int sz=sizeof...(args);
    PyTuple_SetItem(pArgs, index++, Py_BuildValue(((string)arg.des).data(),arg.data));
    set_item(pArgs,index,args...);
}
template <class... T>
PyObject * call_py(string fun_name,PyObject *pDict,T...args )
{
    PyObject*pFunc,*pArgs;
    // printf("sz %d\n",sizeof...(args));
    pFunc = PyDict_GetItemString(pDict, fun_name.data());
    if ( !pFunc || !PyCallable_Check(pFunc) ) {
        printf("can't find function [%s]",fun_name.data());
        // getchar();
        //return -1;
    }
    int sz=sizeof...(args);
    pArgs = PyTuple_New(sz);
    set_item(pArgs,0,args...);
    //    for(int i=0;i<sz;i++)
    //        PyTuple_SetItem(pArgs, i, Py_BuildValue(args[i].des,args[i].data));
    PyObject*ret=PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);
    return ret;
}
PyObject * convert(Mat frame);

class MovidiusProcessor
{
    vector <Rect> result;
    PyObject* obj;
public:
    static MovidiusProcessor &get_instance()
    {
        static MovidiusProcessor pro;
        return pro;
    }

    void process(Mat frame)
    {
        lock.lock();
        result.clear();
        printf("----------- cvt -----------\n");fflush(NULL);
        //  NDArrayConverter cvt;

        //obj = cvt.toNDArray(frame);
        printf("----------- cvt done-----------\n");fflush(NULL);


        py_arg_t<PyObject*> test_arg(convert(frame),"O");

        PyObject* rect_data;

        PyObject* ret_objs;
        rect_data= call_py("process",pDict,test_arg);
        PyArg_Parse(rect_data, "O!", &PyList_Type, &ret_objs);
        int size=PyList_Size(ret_objs);
        printf("-----------get object rects: %d-----------\n",size/4);
        lock.unlock();
    }

    vector <Rect> get_rects()
    {
        return result;
    }

private:
    MovidiusProcessor()
    {
        // Py_Initialize();
        init();
        //         // 检查初始化是否成功
        //         if ( !Py_IsInitialized() ) {
        //          //   return -1;
        //         }
        //         pName = PyString_FromString("run1");
        //         //  pName = PyString_FromString("run");
        //         pModule = PyImport_Import(pName);
        //         if ( !pModule ) {
        //             printf("can't find .py");
        //             getchar();
        //          //   return -1;
        //         }
        //         pDict = PyModule_GetDict(pModule);
        //         if ( !pDict ) {
        //            // return -1;
        //         }



    }
    ~MovidiusProcessor()
    {
        Py_DECREF(pName);
        // Py_DECREF(pArgs);
        Py_DECREF(pModule);
        Py_DECREF(pDict);

        // 关闭Python
        Py_Finalize();
        call_py("release",pDict);
    }

    void init()
    {
        Py_Initialize();
        if ( !Py_IsInitialized() ) {
            printf("init err\n");
        }else{
            printf("init ok\n");
        }
          printf("finding ...\n");

//          PyRun_SimpleString( "import sys");
//          PyRun_SimpleString("sys.path.append('./')");

        pName = PyString_FromString("movidius");


        if(!pName){
            printf("finding err \n");fflush(NULL);
        }else{
            printf("finding ok \n");fflush(NULL);
        }

        pModule = PyImport_Import(pName);
        if ( !pModule ) {
            printf("can't find .py");fflush(NULL);
        }else{
            printf("py found\n");fflush(NULL);
        }
        pDict = PyModule_GetDict(pModule);
        if ( !pDict ) {
            printf("can't find dict");fflush(NULL);
        }else{
               printf("dict found\n");fflush(NULL);
        }
       // call_py("init",pDict);
        call_py("process123",pDict);

    }

    mutex lock;
    PyObject *pName,*pModule,*pDict;

};

#endif // MOVIDIUSPROCESSOR_H
