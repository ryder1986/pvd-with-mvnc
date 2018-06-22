/**g++ -o callpy callpy.cpp -I/usr/include/python2.6 -L/usr/lib64/python2.6/config -lpython2.6**/  
#include </usr/include/python2.7/Python.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/objdetect/objdetect.hpp>
using namespace cv;
#include "conversion.h"
using namespace std;
using namespace cv;
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
        return NULL;
    }
    int sz=sizeof...(args);
    pArgs = PyTuple_New(sz);
    set_item(pArgs,0,args...);
    //    for(int i=0;i<sz;i++)
    //        PyTuple_SetItem(pArgs, i, Py_BuildValue(args[i].des,args[i].data));
    PyObject *ret_obj;
    ret_obj=PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);
    return ret_obj;
}
//NumpyAllocator g_numpyAllocator;
////以Mat的allocator作为基类，Numpy的Allocator作为继承类
////这样可以用派生对象指针对基类数据进行操作
//class NumpyAllocator : public MatAllocator
//{
//public:
//    NumpyAllocator() { stdAllocator = Mat::getStdAllocator(); }
//    ~NumpyAllocator() {}

//    UMatData* allocate(PyObject* o, int dims, const int* sizes, int type, size_t* step) const
//    {
//        UMatData* u = new UMatData(this);
//        u->data = u->origdata = (uchar*)PyArray_DATA((PyArrayObject*) o);
//        npy_intp* _strides = PyArray_STRIDES((PyArrayObject*) o);
//        for( int i = 0; i < dims - 1; i++ )
//            step[i] = (size_t)_strides[i];
//        step[dims-1] = CV_ELEM_SIZE(type);
//        u->size = sizes[0]*step[0];
//        u->userdata = o;
//        return u;
//    }

//    UMatData* allocate(int dims0, const int* sizes, int type, void* data, size_t* step, int flags, UMatUsageFlags usageFlags) const
//    {
//        if( data != 0 )
//        {
//            CV_Error(Error::StsAssert, "The data should normally be NULL!");
//            // probably this is safe to do in such extreme case
//            return stdAllocator->allocate(dims0, sizes, type, data, step, flags, usageFlags);
//        }
//        //确保当前使用python的C API是线程安全的
//        PyEnsureGIL gil;

//        int depth = CV_MAT_DEPTH(type);
//        int cn = CV_MAT_CN(type);
//        const int f = (int)(sizeof(size_t)/8);
//        int typenum = depth == CV_8U ? NPY_UBYTE : depth == CV_8S ? NPY_BYTE :
//        depth == CV_16U ? NPY_USHORT : depth == CV_16S ? NPY_SHORT :
//        depth == CV_32S ? NPY_INT : depth == CV_32F ? NPY_FLOAT :
//        depth == CV_64F ? NPY_DOUBLE : f*NPY_ULONGLONG + (f^1)*NPY_UINT;
//        int i, dims = dims0;
//        cv::AutoBuffer<npy_intp> _sizes(dims + 1);
//        for( i = 0; i < dims; i++ )
//            _sizes[i] = sizes[i];
//        if( cn > 1 )
//            _sizes[dims++] = cn;
//        PyObject* o = PyArray_SimpleNew(dims, _sizes, typenum);
//        if(!o)
//            CV_Error_(Error::StsError, ("The numpy array of typenum=%d, ndims=%d can not be created", typenum, dims));
//        return allocate(o, dims0, sizes, type, step);
//    }

//    bool allocate(UMatData* u, int accessFlags, UMatUsageFlags usageFlags) const
//    {
//        return stdAllocator->allocate(u, accessFlags, usageFlags);
//    }

//    void deallocate(UMatData* u) const
//    {
//        if(!u)
//            return;
//        PyEnsureGIL gil;
//        CV_Assert(u->urefcount >= 0);
//        CV_Assert(u->refcount >= 0);
//        if(u->refcount == 0)
//        {
//            PyObject* o = (PyObject*)u->userdata;
//            Py_XDECREF(o);
//            delete u;
//        }
//    }
//    //基类指针，调用allocate函数进行内存分配
//    const MatAllocator* stdAllocator;
//};
//PyObject* fromMatToNDArray(const Mat& m) {
//    if( !m.data )
//        Py_RETURN_NONE;
//    Mat temp, *p = (Mat*)&m;
//    if(!p->refcount || p->allocator != &g_numpyAllocator)
//    {
//        temp.allocator = &g_numpyAllocator;
//        ERRWRAP2(m.copyTo(temp));
//        p = &temp;
//    }
//    p->addref();
//    return pyObjectFromRefcount(p->refcount);
//}

int main(int argc, char** argv)  
{  
    // 初始化Python
    //在使用Python系统前，必须使用Py_Initialize对其
    //进行初始化。它会载入Python的内建模块并添加系统路
    //径到模块搜索路径中。这个函数没有返回值，检查系统
    //是否初始化成功需要使用Py_IsInitialized。
    Py_Initialize();

    // 检查初始化是否成功
    if ( !Py_IsInitialized() ) {
        return -1;
    }
    // 添加当前路径
    //把输入的字符串作为Python代码直接运行，返回0
    //表示成功，-1表示有错。大多时候错误都是因为字符串
    //中有语法错误。
    //  PyRun_SimpleString("import sys");
    // PyRun_SimpleString("print '---import sys---'");
    //  PyRun_SimpleString("sys.path.append('./')");




    PyObject *pName,*pModule,*pDict,*pFunc;

    // 载入名为pytest的脚本
    //pName = PyString_FromString("pytest");
    pName = PyString_FromString("movidius");
    //  pName = PyString_FromString("run");
    pModule = PyImport_Import(pName);
    if ( !pModule ) {
        printf("can't find .py");
        getchar();
        return -1;
    }
    pDict = PyModule_GetDict(pModule);
    if ( !pDict ) {
        return -1;
    }

#if 0
    NDArrayConverter cvt;
    VideoCapture vcap( "rtsp://192.168.1.80:554/av0_1");
    if(!vcap.isOpened()){
        cout<<"er";
        return -1;
    }else{
        cout<<"ok";
    }
    Mat  mat_rst;
    bool rt= vcap.read(mat_rst);
    if(rt){
        cout<<"read a frame";
    }
    PyObject* obj = cvt.toNDArray(mat_rst);









#endif




    //     py_arg_t<int> a[2];
    //     a[0].data=1;
    //     a[1].data=2;
    //     a[0].des="l";
    //     a[1].des="l";

    //    py_arg_t<int> a1(1,"l");
    //    py_arg_t<int> a2(3,"l");
    //    //call_py("add",pDict,a1,a2);
    //    py_arg_t<float> a3(8.0,"f");
    //    py_arg_t<PyObject*> mt(obj,"o");
    //    call_py("foo",pDict,a3);
    //call_py("main_test",pDict);




    //    call_py("init",pDict);

    //     while(1)
    //     call_py("run_pic1",pDict,mt);
    //     call_py("release",pDict);


    //    py_arg_t<PyObject*> test_arg(obj,"O");

    //    PyObject* rst=call_py("test_arg",pDict,test_arg);







    //    PyObject* rst=call_py("fun123",pDict);

    //    int ret_v=0;
    //    PyArg_Parse(rst, "i", &ret_v);


    //    printf("------ret %d----------------\n",ret_v);






    //  PyObject* obj1;


    //    //int ret_v=0;
    //    //PyArg_Parse(rst, "i", &ret_v);
    //    PyObject* ret_obj=call_py("fun4",pDict);
    //  // PyTupleObject rst(1,2,3,4);
    //   static  int left, top, h, v;
    ////     PyArg_ParseTuple(ret_obj, "iiii",
    ////             &left, &top, &h, &v);
    ////     PyArg_ParseTuple(ret_obj, "iiii",
    ////             &left, &top, &h, &v);
    //    /* A rectangle and a point */
    //    /* Possible Python call:
    //       f(((0, 0), (400, 300)), (10, 10)) */


    //     PyArg_ParseTuple(ret_obj, "O!",
    //             obj1);

    //   //  int n = PyList_Size(pList);
    //     int n = PyTuple_Size(obj1);

    //    printf("------sz %d-------------\n", n);



    //    PyObject* ret_obj=call_py("fun5",pDict);

    //    PyObject *pTuple;
    //    PyObject *pItem;
    //    Py_ssize_t n;
    //    int i;


    //    PyObject *element1 = PyTuple_GetItem(ret_obj, 5);
    //    int ret_v=0;
    //    PyArg_Parse(element1, "i", &ret_v);
    //    printf("------ret %d-------------\n", ret_v);


    // if (!PyArg_ParseTuple(ret_obj, "O!", &PyTuple_Type, &pList)) {
    //    if (!PyArg_ParseTuple(ret_obj, "O!",&PyTuple_Type ,&pTuple)) {
    //        printf("----err--sz %d-------------\n", n);

    //     //   PyErr_SetString(PyExc_TypeError, "parameter must be a list.");
    //    //    return NULL;
    //    }
    //    n = PyTuple_Size(pTuple);
    //    printf("------sz %d-------------\n", n);

    //    n = PyList_Size(pList);
    //    for (i=0; i<n; i++) {
    //        pItem = PyList_GetItem(pList, i);
    //        if(!PyInt_Check(pItem)) {
    //            PyErr_SetString(PyExc_TypeError, "list items must be integers.");
    //            return NULL;
    //        }
    //    }
    //PyArg_ParseTuple(ret_obj, "O!",&PyTuple_Type ,&pTuple);



    //    PyArg_ParseTuple(ret_obj, "O!", &PyTuple_Type, &pTuple);



    //    n = PyTuple_Size(pTuple);
    //    printf("------sz %d-------------\n", n);


    PyObject *pList6;
    PyObject* ret_obj6=call_py("fun6",pDict);
    PyArg_Parse(ret_obj6, "O!", &PyList_Type, &pList6);
    //   PyListObject *lst=pList6;

    int n6=PyList_Size(pList6);
    printf("-----  %d-------------\n", n6);
    int tmp_v;
    PyObject *element;
    for(int i=0;i<n6;i++){

        element = PyList_GetItem(pList6, i);
        PyArg_Parse(element, "i", &tmp_v);
        printf("-----------%d-----------\n",tmp_v);

    }







    NDArrayConverter cvt;
    VideoCapture vcap( "rtsp://192.168.1.80:554/av0_1");
    if(!vcap.isOpened()){
        cout<<"er";
        return -1;
    }else{
        cout<<"ok";
    }
    Mat  mat_rst;
    //    bool rt= vcap.read(mat_rst);
    //    if(rt){
    //        cout<<"read a frame";
    //    }
    //    PyObject* obj = cvt.toNDArray(mat_rst);
    //    py_arg_t<PyObject*> mt(obj,"O");
    PyObject* obj;

    call_py("init",pDict);
    PyObject *ret;
    PyObject *ret_objs;
    int size;
    while(1){
        bool rt= vcap.read(mat_rst);
        if(rt){
            cout<<"read a frame";
        }
        obj = cvt.toNDArray(mat_rst);
        py_arg_t<PyObject*> mt(obj,"O");

        ret=call_py("run_1_pic",pDict,mt);

        PyArg_Parse(ret, "O!", &PyList_Type, &ret_objs);
        size=PyList_Size(ret_objs);
        printf("-----------get object rects: %d-----------\n",size/4);
    }
    //    call_py("release",pDict);



#if 0
    // 找出函数名为add的函数
    printf("----------------------\n");
    pFunc = PyDict_GetItemString(pDict, "add");
    if ( !pFunc || !PyCallable_Check(pFunc) ) {
        printf("can't find function [add]");
        getchar();
        return -1;
    }

    // 参数进栈
    *pArgs;
    pArgs = PyTuple_New(2);

    //  PyObject* Py_BuildValue(char *format, ...)
    //  把C++的变量转换成一个Python对象。当需要从
    //  C++传递变量到Python时，就会使用这个函数。此函数
    //  有点类似C的printf，但格式不同。常用的格式有
    //  s 表示字符串，
    //  i 表示整型变量，
    //  f 表示浮点数，
    //  O 表示一个Python对象。

    PyTuple_SetItem(pArgs, 0, Py_BuildValue("l",3));
    PyTuple_SetItem(pArgs, 1, Py_BuildValue("l",4));

    // 调用Python函数
    PyObject_CallObject(pFunc, pArgs);




    /////


    //下面这段是查找函数foo 并执行foo
    printf("----------------------\n");
    pFunc = PyDict_GetItemString(pDict, "foo");
    if ( !pFunc || !PyCallable_Check(pFunc) ) {
        printf("can't find function [foo]");
        getchar();
        return -1;
    }
    pArgs = PyTuple_New(1);
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("l",2));

    PyObject_CallObject(pFunc, pArgs);

    printf("----------------------\n");



    //////////
    pFunc = PyDict_GetItemString(pDict, "update");
    if ( !pFunc || !PyCallable_Check(pFunc) ) {
        printf("can't find function [update]");
        getchar();
        return -1;
    }
    pArgs = PyTuple_New(0);
    PyTuple_SetItem(pArgs, 0, Py_BuildValue(""));
    PyObject_CallObject(pFunc, pArgs);



#endif
    //////
    Py_DECREF(pName);
    // Py_DECREF(pArgs);
    Py_DECREF(pModule);

    // 关闭Python
    Py_Finalize();
    return 0;
}
