/* Copyright 2015, 2016 Gerald Baier
 *
 * This file is part of despeckCL.
 *
 * despeckCL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * despeckCL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with despeckCL. If not, see <http://www.gnu.org/licenses/>.
 */

__kernel void covmat_spatial_avg (__global float * covmat_in,
                                  __global float * covmat_out, 
                                  const int dimension,
                                  const int height_overlap,
                                  const int width_overlap,
                                  const int scale_size,
                                  const int scale_size_max,
                                  __local float * local_data,
                                  __constant float * gauss)
{
    const int delta_scale = (scale_size_max -  scale_size)/2;

    const int block_size = get_local_size(0);
    const int output_block_size = get_local_size(0) - scale_size + 1;

    const int tx = get_local_id(0);
    const int ty = get_local_id(1);

    const int in_x = get_group_id(0) * output_block_size + tx;
    const int in_y = get_group_id(1) * output_block_size + ty;

    const int height_overlap_avg = height_overlap + scale_size_max - 1;
    const int width_overlap_avg  = width_overlap  + scale_size_max - 1;

    for(int i = 0; i < 2*dimension*dimension; i++) {
        if ( (in_x < height_overlap_avg) && (in_y < width_overlap_avg) ) {
            local_data [tx*block_size + ty] = covmat_in [i*height_overlap_avg*width_overlap_avg + (delta_scale + in_x) *width_overlap_avg + delta_scale + in_y];
        }

        barrier(CLK_LOCAL_MEM_FENCE);

        float sum = 0;

        if ((tx < output_block_size) && (ty < output_block_size)) {
            for(int kx = 0; kx < scale_size; kx++) {
                for(int ky = 0; ky < scale_size; ky++) {
                    sum += gauss[kx*scale_size + ky] * local_data[(tx + kx)*block_size + ty + ky];
                }
            }
            if (in_x < height_overlap &&  in_y < width_overlap) {
                covmat_out[i*height_overlap*width_overlap + in_x*width_overlap + in_y] = sum;
            }
        }
    }
}
