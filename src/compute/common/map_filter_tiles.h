#ifndef MAP_FILTER_TILES_H
#define MAP_FILTER_TILES_H

#include <vector>

#include <omp.h>

#include "data.h"
#include "logging.h"
#include "tile_iterator.h"
#include "tile.h"

namespace boxcar {
    struct cl_wrappers;
    struct kernel_params;;
    cl_wrappers get_cl_wrappers(cl::Context cl_context, kernel_params pm);
};

namespace nlsar {
    struct cl_wrappers;
    struct kernel_params;;
    cl_wrappers get_cl_wrappers(cl::Context cl_context, kernel_params pm);
}

namespace nlinsar{
    struct cl_wrappers;
    struct kernel_params;;
    cl_wrappers get_cl_wrappers(cl::Context cl_context, kernel_params pm);
}

namespace goldstein{
    struct cl_wrappers;
    struct kernel_params;;
    cl_wrappers get_cl_wrappers(cl::Context cl_context, kernel_params pm);
}

template <typename Type, typename Filter, typename Kernelparams, typename... Params>
timings::map
map_filter_tiles(Filter func,
                 Type& total_image_in,
                 Type& total_image_out,
                 Kernelparams kernel_params,
                 std::pair<int, int> tile_dims,
                 int overlap,
                 Params... parameters)
{
  // for each device a context and kernels were created
  auto cl_devs = get_platform_devs(0);
  int n_devices = cl_devs.size();

  std::vector<cl::Context> cl_contexts;
  std::vector<decltype(get_cl_wrappers(cl::Context(), kernel_params))> cl_routiness;

  for(const auto& dev : cl_devs) {
      auto cl_context = cl::Context{dev};
      auto cl_routines(get_cl_wrappers(cl_context, kernel_params));
      cl_contexts.push_back(cl_context);
      cl_routiness.push_back(cl_routines);
  }

  omp_set_num_threads(2*n_devices);
  timings::map tm;
  LOG(INFO) << "starting filtering";
#pragma omp parallel shared(total_image_in, total_image_out)
  {
#pragma omp master
    {
      LOG(INFO) << "using " << omp_get_num_threads() << " threads for " << n_devices << " GPUs";
      for (auto t : tile_iterator(total_image_in.height(),
                                  total_image_in.width(),
                                  tile_dims.first,
                                  tile_dims.second,
                                  overlap,
                                  overlap)) {
#pragma omp task firstprivate(t)
        {
          Type imgtile(tileget(total_image_in, t));
          const int dev_idx = omp_get_thread_num() % cl_devs.size();
          auto threadprivate_cl_context{cl_contexts[dev_idx]};
          auto threadprivate_cl_routines{cl_routiness[dev_idx]};
          try {
            timings::map tm_sub =
                func(threadprivate_cl_context, threadprivate_cl_routines, imgtile, parameters...);
#pragma omp critical
            tm = timings::join(tm, tm_sub);
          } catch (cl::Error &error) {
            LOG(ERROR) << error.what() << "(" << error.err() << ")";
            LOG(ERROR) << "ERR while filtering sub image";
            std::terminate();
          }
          tile<2> rel_sub{t[0].get_sub(overlap, -overlap),
                          t[1].get_sub(overlap, -overlap)};
          tile<2> tsub{slice{overlap, imgtile.height() - overlap},
                       slice{overlap, imgtile.width() - overlap}};
          Type imgtile_sub(tileget(imgtile, tsub));
          tilecpy(total_image_out, imgtile_sub, rel_sub);
        }
      }
#pragma omp taskwait
    }
  }
  LOG(INFO) << "filtering done";
  return tm;
}

#endif
