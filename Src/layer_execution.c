
#include "main.h"
#include "network_desc.h"
#include "input.h"
#include "weights.h"
#include "output.h"

int layer_init()
{
#if defined(DEBUG)
  printf("-> Entering Layer Initialization...\n");
#endif

  network_layers[0].input_data = input;
  if(network_layers[0].input_data == NULL)
  {
    return -L2_ERROR;
  }

  network_layers[0].param_data = weights;
  if(network_layers[0].param_data == NULL)
  {
    return -L2_ERROR;
  }

  network_layers[0].output_data = pi_l2_malloc((unsigned int)(network_layers[0].layer_dim.x_out * network_layers[0].layer_dim.y_out * network_layers[0].layer_dim.c_out * sizeof(unsigned char)));
  if(network_layers[0].output_data == NULL)
  {
    return -L2_ERROR;
  }

#if defined(DEBUG)
  printf("-> Exiting Layer Initialization...\n");
#endif

  return 0;
}

/*
 * Cluster application entry point
 */
void layer_run()
{
#if defined(DEBUG)
  printf("--> Entering Layer Running...\n");
#endif

  pi_cl_dma_cmd_t dma_cmd;

  int dma_copy_size = l1_layer.layer_dim.x_ker * l1_layer.layer_dim.y_ker * l1_layer.layer_dim.c_in * l1_layer.layer_dim.c_out;

  /*
   * Parameters copy: Let's keep it simple, avoid the tiling along channels. Move all the parameters in L1
   */
#ifndef EXERCISE2
  pi_cl_dma_cmd((unsigned int)network_layers[0].param_data, (unsigned int)l1_layer.param_data, dma_copy_size, PI_CL_DMA_DIR_EXT2LOC, &dma_cmd);
  pi_cl_dma_cmd_wait(&dma_cmd);
#else
  l1_layer.param_data = network_layers[0].param_data;
#endif

#ifdef EXERCISE2
  l1_layer.output_data = network_layers[0].output_data;
#endif
  /*
   * Tile loop bounds
   */
  int nb_h_tile = 0;
  int nb_w_tile = 0;

  /*
   * Tile loop indexes
   */
  int h_tile = 0;
  int w_tile = 0;

  /*
   * Cluster performance counters
   */
#if defined(PERFORMANCE)
  /* COMPLETE EX1/2/3 WITH PERF COUNTER INIT */
#endif
  
#ifdef EXERCISE3
  nb_h_tile = /* COMPLETE EX3 WITH NUMBER OF TILES ON H DIM */;
  nb_w_tile = /* COMPLETE EX3 WITH NUMBER OF TILES ON W DIM */;
#else
  nb_h_tile = 1;
  nb_w_tile = 1;
#endif 

  for(h_tile=0; h_tile < nb_h_tile; h_tile++) 
  {
    for(w_tile=0; w_tile < nb_w_tile; w_tile++) 
    {
      /*
       * Kernel initialization
       */
      kernel_init(h_tile, w_tile, 0);

      /*
       * Executing the main kernel
       */
      kernel_run();


      /*
       * Kernel ending
       */
      kernel_end(h_tile, w_tile, 0);
    }
  }

#if defined(PERFORMANCE)
  /* COMPLETE EX1/2/3 WITH PERF COUNTER READING */
  uint32_t instr_cnt      = /* COMPLETE EX1/2/3 */;
  uint32_t cycles_cnt     = /* COMPLETE EX1/2/3 */;
  printf("[0]: instructions = %d, tot_cycles = %d\n", instr_cnt, cycles_cnt);
#endif

#if defined(DEBUG)
  printf("--> Exiting Layer Running...\n");
#endif
}

/*
 * Check if the outputs are the same of the golden model
 */
int layer_check()
{
  int tot_layer_out_dim = network_layers[0].layer_dim.x_out * network_layers[0].layer_dim.y_out * network_layers[0].layer_dim.c_out;

  for(int i=0; i<tot_layer_out_dim; i++)
  {
    if(output[i] != network_layers[0].output_data[i])
    {
      printf("ERROR at index %d, expected %x and got %x\n", i, output[i], network_layers[0].output_data[i]);
      return -GENERAL_ERROR;
    }
  }

  printf("Exiting layer with 0 errors\n");

  return 0;
}

int layer_free()
{
#ifndef EXERCISE2
  pi_l1_free(&cluster, l1_layer.input_data , (unsigned int)(l1_layer.layer_dim.x_in  * l1_layer.layer_dim.y_in  * l1_layer.layer_dim.c_in * sizeof(unsigned char)));
  pi_l1_free(&cluster, l1_layer.param_data , (unsigned int)(l1_layer.layer_dim.x_ker * l1_layer.layer_dim.y_ker * l1_layer.layer_dim.c_in * l1_layer.layer_dim.c_out * sizeof(signed char)));
  pi_l1_free(&cluster, l1_layer.output_data, (unsigned int)(l1_layer.layer_dim.x_out * l1_layer.layer_dim.y_out * l1_layer.layer_dim.c_out * sizeof(unsigned char)));
#endif
  pi_cluster_close(&cluster);
  return 0;
}