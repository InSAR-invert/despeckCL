#include "covmat_create.h"

void nlsar::covmat_create::run(cl::CommandQueue cmd_queue,
                               cl::Buffer ampl_master,
                               cl::Buffer ampl_slave,
                               cl::Buffer dphase,
                               cl::Buffer covmat,
                               const int height,
                               const int width)
{
    kernel.setArg(0, ampl_master);
    kernel.setArg(1, ampl_slave);
    kernel.setArg(2, dphase);
    kernel.setArg(3, covmat);
    kernel.setArg(4, height);
    kernel.setArg(5, width);

    cl::NDRange global_size {(size_t) block_size*( (height - 1)/block_size + 1), \
                             (size_t) block_size*( (width  - 1)/block_size + 1)};
    cl::NDRange local_size  {block_size, block_size};

    cmd_queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_size, local_size, NULL, NULL);
}
