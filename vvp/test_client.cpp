#include <iostream>
#include <vpi_user.h>

void compile();

void cpp_memread_register();

static int hello_compiletf(char *) {
    return 0;
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

const int MAX_KERNEL_SIZE = 8;
int kernel_size = 8;
int kernel[MAX_KERNEL_SIZE + 1];

void cpp_memread(int i, int *p_mem_req, int *p_mem_addr, int *p_mem_data)   
{
    *p_mem_req = 0;
    *p_mem_addr = 0;
    *p_mem_data = 0;
    if(i < kernel_size) {
        *p_mem_req = 1;
        *p_mem_addr = i;
        *p_mem_data = kernel[i];
    }
}

int main(int argc, char *argv[]) {
    cpp_memread_register();

    kernel[0] = 111;
    kernel[1] = 321;
    kernel[2] = 444;
    kernel_size = 3;

    compile();
    return 0;
}
