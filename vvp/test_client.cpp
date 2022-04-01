#include <iostream>
#include <vpi_user.h>

void compile();

void hello_register();
void giveme5_register();
void cpp_memread_register();

static int hello_compiletf(char *) {
    return 0;
}


void hello();

static int hello_calltf(char *)
{
    hello();
    return 0;
}

void hello_register() {
    s_vpi_systf_data tf_data;

    tf_data.type = vpiSysTask;
    tf_data.tfname = "$hello";
    tf_data.calltf = hello_calltf;
    tf_data.compiletf= hello_compiletf;
    tf_data.sizetf = 0;
    tf_data.user_data = 0;
    vpi_register_systf(&tf_data);
}

static int giveme5_compiletf(char *)
{
    return 0;
}

int giveme5(int input);

static int giveme5_calltf(char *)
{
    vpiHandle systfref, args_iter, arg0, arg1;
    struct t_vpi_value argval;
    // vpi_printf("hello, world\n");
    systfref = vpi_handle(vpiSysTfCall, NULL);
    args_iter = vpi_iterate(vpiArgument, systfref);
    arg0 = vpi_scan(args_iter);
    arg1 = vpi_scan(args_iter);
    vpi_free_object(args_iter);

    argval.format = vpiIntVal;
    vpi_get_value(arg0, &argval);

    int output = giveme5(argval.value.integer);

    // std::cout << argval.value.integer << std::endl;

    argval.value.integer = output;
    vpi_put_value(arg1, &argval, NULL, vpiNoDelay);

    return 0;
}

void giveme5_register()
{
    s_vpi_systf_data tf_data;

    tf_data.type = vpiSysTask;
    // tf_data.type = vpiSysFunc;
    // tf_data.sysfunctype = vpiSysFuncSized 32 unsigned;
    tf_data.tfname = "$giveme5";
    tf_data.calltf = giveme5_calltf;
    tf_data.compiletf = giveme5_compiletf;
    tf_data.sizetf = 0;
    tf_data.user_data = 0;
    vpi_register_systf(&tf_data);
}

void cpp_memread(int i, int *p_mem_req, int *p_mem_addr, int *p_mem_data);

static int cpp_memread_calltf(char *)
{
    vpiHandle systfref, args_iter, arg0, arg1, arg2, arg3;
    struct t_vpi_value argval;
    // vpi_printf("hello, world\n");
    systfref = vpi_handle(vpiSysTfCall, NULL);
    args_iter = vpi_iterate(vpiArgument, systfref);
    arg0 = vpi_scan(args_iter);
    arg1 = vpi_scan(args_iter);
    arg2 = vpi_scan(args_iter);
    arg3 = vpi_scan(args_iter);
    vpi_free_object(args_iter);

    argval.format = vpiIntVal;
    vpi_get_value(arg0, &argval);

    int i = argval.value.integer;
    int req, addr, out;
    cpp_memread(i, &req, &addr, &out);

    // std::cout << argval.value.integer << std::endl;

    argval.value.integer = req;
    vpi_put_value(arg1, &argval, NULL, vpiNoDelay);
    argval.value.integer = addr;
    vpi_put_value(arg2, &argval, NULL, vpiNoDelay);
    argval.value.integer = out;
    vpi_put_value(arg3, &argval, NULL, vpiNoDelay);

    return 0;
}

static int cpp_memread_compiletf(char *)
{
    return 0;
}

void cpp_memread_register()
{
    s_vpi_systf_data tf_data;

    tf_data.type = vpiSysTask;
    // tf_data.type = vpiSysFunc;
    // tf_data.sysfunctype = vpiSysFuncSized 32 unsigned;
    tf_data.tfname = "$cpp_memread";
    tf_data.calltf = cpp_memread_calltf;
    tf_data.compiletf = cpp_memread_compiletf;
    tf_data.sizetf = 0;
    tf_data.user_data = 0;
    vpi_register_systf(&tf_data);
}

void hello() {
    vpi_printf("hello, world\n");
}

int giveme5(int input) {
    std::cout << input << std::endl;
    unsigned int output;
    output = 5;
    output = 4294967295;
    output = -123;
    return output;
}

void cpp_memread(int i, int *p_mem_req, int *p_mem_addr, int *p_mem_data)   
{
    *p_mem_req = 0;
    *p_mem_addr = 0;
    *p_mem_data = 0;
    switch(i) {
        case 0:
            *p_mem_req = 1;
            *p_mem_addr = 1;
            *p_mem_data = 222;
            break;
        case 1:
            *p_mem_req = 1;
            *p_mem_addr = 2;
            *p_mem_data = 333;
            break;
        case 2:
            *p_mem_req = 1;
            *p_mem_addr = 3;
            *p_mem_data = 444;
            break;
        case 3:
            *p_mem_req = 1;
            *p_mem_addr = 5;
            *p_mem_data = 555;
            break;
    }
}

int main(int argc, char *argv[]) {
    hello_register();
    giveme5_register();
    cpp_memread_register();
    compile();
    return 0;
}
