// Copyright 2021 DeepMind Technologies Limited
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdbool.h> //for bool
//#include<unistd.h> //for usleep
//#include <math.h>
#include <mujoco/mujoco.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <GLFW/glfw3.h>
#include <time.h>

#define ndof 1

// display simulation window - true / reccord simulation & create mp4 - false
bool display_window = true;

// xml & video files
char xmlpath[] = "pendulum.xml";
char rgb_file[] = "./pend.out";

//related to writing simulation data to a file
bool record_data = false;
FILE *fid;
int loop_index = 0;
const int data_frequency = 1; //frequency at which data is written to a file

//simulation end time
int simend = 20; // ensure that simend > video_length
double t_control = 0;

// video_length & fps for recording
int video_length = 10; // ensure that video_length < simend
const int fps = 60;

// controller related variables
float_t ctrl_update_freq = 100;
mjtNum last_update = 0.0;
mjtNum ctrl;

// MuJoCo data structures
mjModel* m = NULL;                  // MuJoCo model
mjData* d = NULL;                   // MuJoCo data
mjvCamera cam;                      // abstract camera
mjvOption opt;                      // visualization options
mjvScene scn;                       // abstract scene
mjrContext con;                     // custom GPU context

// mouse interaction
bool button_left = false;
bool button_middle = false;
bool button_right =  false;
double lastx = 0;
double lasty = 0;

// holders of one step history of time and position to calculate dertivatives
mjtNum position_history = 0;
mjtNum previous_time = 0;

typedef struct data_array{
  double cam_yaw;
  double cam_pitch;
} Data;

// double* campos_traj = {0};


// ex time_arr
// cam_time_arr[3][2] = {{0 1},{1 5},{5 10}};

// ex cam_pos_arr
// cam_pos_arr[4][2] = {{90 90},{-180 90},{-180 0},{90 90}};

// double * cam_ref_arr(double * cam_time_arr[], double * cam_pos_arr[], double current_campos[6])
// {
//   int tspan[][1] = {0};
//   int t_size = sizeof(*cam_time_arr) / 2;
//   int t_total = 0;
//
//   for (int i=0;i<t_size;i++){
//     tspan[i][0] = (cam_time_arr[i][1] - cam_time_arr[i][0])*fps;
//     t_total = t_total + tspan[i][0];
//   }
//
//   int cam_size = (sizeof(*cam_pos_arr) / 2) - 1;
//   double dcampos_yaw[cam_size];
//   double dcampos_pitch[cam_size];
//   memset( dcampos_yaw, 0, cam_size*sizeof(double) );
//   memset( dcampos_pitch, 0, cam_size*sizeof(double) );
//
//
//   for (int i=0;i<cam_size;i++){
//     dcampos_yaw[i] = (cam_pos_arr[i+1][0] - cam_pos_arr[i][0]) / (tspan[i][0]*fps);
//     dcampos_pitch[i] = (cam_pos_arr[i+1][1] - cam_pos_arr[i][1]) / (tspan[i][0]*fps);
//   }
//
//   int count = 0;
//   double campos_traj[t_total][2];
//
//   campos_traj[0][0] = current_campos[0];
//   campos_traj[0][1] = current_campos[1];
//
//   int t = 0;
//
//   for (int j=0;j<t_size;j++){
//     while (count <= tspan[j][0]){
//       campos_traj[t+1][0] = campos_traj[t][0] + dcampos_yaw[j];
//       campos_traj[t+1][1] = campos_traj[t][1] + dcampos_pitch[j];
//       count += count;
//       t += 1;
//     }
//     count = 0;
//   }
//
//   return campos_traj;
// }


//This function is called once and is used to get the headers
void init_save_data()
{
  time_t timer;
  char buffer[48];
  struct tm* tm_info;

  timer = time(NULL);
  tm_info = localtime(&timer);

  strftime(buffer, 48, "data_%Y_%m_%d %H:%M:%S.csv", tm_info);

  fid = fopen(buffer,"w");
  //write name of the variable here (header)
  fprintf(fid,"t,");
  fprintf(fid,"Qpitch,Vpitch,Tpitch,Force,");

  //Don't remove the newline
  fprintf(fid,"\n");
}


//This function is called at a set frequency, put data here
void save_data(const mjModel* m, mjData* d)
{

  //data here should correspond to headers in init_save_data()
  //seperate data by a space %f followed by space
  fprintf(fid,"%f,",d->time);

  fprintf(fid,"%f,",d->qpos[0]);
  fprintf(fid,"%f,",d->qvel[0]);
  fprintf(fid,"%f,",d->ctrl[0]);
  fprintf(fid,"%f,",d->xfrc_applied[6]);
  //Don't remove the newline
  fprintf(fid,"\n");
}


void mycontroller(const mjModel* m, mjData* d)
{
  //write control here
  mj_energyPos(m,d);
  mj_energyVel(m,d);
  //printf("%f %f %f %f \n",d->time,d->energy[0],d->energy[1],d->energy[0]+d->energy[1]);

  //check equations
  //M*qacc + qfrc_bias = qfrc_applied + ctrl
  //M*qddot + f = qfrc_applied + ctrl
  double dense_M[ndof*ndof] = {0};
  mj_fullM(m,dense_M, d->qM);
  double M={0};
  M = dense_M[0];

  double qddot[ndof]={0};
  qddot[0]=d->qacc[0];

  double f[ndof]={0};
  f[0] = d->qfrc_bias[0];

  double lhs[ndof]={0};
  mju_mulMatVec(lhs,dense_M,qddot,1,1); //lhs = M*qddot
  lhs[0] = lhs[0] + f[0]; //lhs = M*qddot + f

  //control
  double Kp1 = 900;
  double Kv1 = 200;
  double qref1 = 0;

  //PD control
  d->qfrc_applied[0] = -Kp1*(d->qpos[0]-qref1)-Kv1*d->qvel[0];

  double rhs[ndof]={0};
  rhs[0] = d->qfrc_applied[0];
}


void apply_forces(const mjModel* m, mjData* d)
{
  //  random disturbance at every time step
  double t = d->time;
  int i = 1;
  int count = 0;

  if (t>1)
  {
    count = count + 1;
    if (count % 1 == 0)
    {
    double noise_Fx, noise_Fy;
    double Fx_world, Fy_world;

    mju_standardNormal(&noise_Fx);
    double Fx_max = 1500.0;
    double Fx =  Fx_max*noise_Fx;
    Fx_world = Fx;//*c-Fy*s;

    mju_standardNormal(&noise_Fy);
    double Fy_max = 0;
    double Fy = Fy_max*noise_Fy;
    Fy_world = Fy;//Fx*s+Fy*c;

    d->xfrc_applied[6*i+0] = Fx_world;
    d->xfrc_applied[6*i+1] = Fy_world;
    }
  }
}

// control loop callback
void loop(const mjModel* m, mjData* d)
{

  int i;

  mycontroller(m,d); //control loop

  apply_forces(m,d); // apply forces

  if (record_data){
    if ( loop_index%data_frequency==0)
    {
      save_data(m,d);
    }
    loop_index = loop_index + 1;
  }

}


//-------------------------------- utility functions ------------------------------------

// keyboard callback
void keyboard(GLFWwindow* window, int key, int scancode, int act, int mods)
{
    // backspace: reset simulation
    if( act==GLFW_PRESS && key==GLFW_KEY_BACKSPACE )
    {
        mj_resetData(m, d);
        mj_forward(m, d);
    }
}

// mouse button callback
void mouse_button(GLFWwindow* window, int button, int act, int mods)
{
    // update button state
    button_left =   (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS);
    button_middle = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE)==GLFW_PRESS);
    button_right =  (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS);

    // update mouse position
    glfwGetCursorPos(window, &lastx, &lasty);
}


// mouse move callback
void mouse_move(GLFWwindow* window, double xpos, double ypos)
{
    // no buttons down: nothing to do
    if( !button_left && !button_middle && !button_right )
        return;

    // compute mouse displacement, save
    double dx = xpos - lastx;
    double dy = ypos - lasty;
    lastx = xpos;
    lasty = ypos;

    // get current window size
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // get shift key state
    bool mod_shift = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS ||
                      glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT)==GLFW_PRESS);

    // determine action based on mouse button
    mjtMouse action;
    if( button_right )
        action = mod_shift ? mjMOUSE_MOVE_H : mjMOUSE_MOVE_V;
    else if( button_left )
        action = mod_shift ? mjMOUSE_ROTATE_H : mjMOUSE_ROTATE_V;
    else
        action = mjMOUSE_ZOOM;

    // move camera
    mjv_moveCamera(m, action, dx/height, dy/height, &scn, &cam);
}


// scroll callback
void scroll(GLFWwindow* window, double xoffset, double yoffset)
{
    // emulate vertical mouse motion = 5% of window height
    mjv_moveCamera(m, mjMOUSE_ZOOM, 0, -0.05*yoffset, &scn, &cam);
}



//-------------------------------- main function ----------------------------------------
int main(int argc, const char** argv)
{
  // used to record simulation, otherwise will be left empty
  FILE* fp;
  unsigned char* rgb;
  float* depth;
  double frametime = 0;
  int framecount = 0;

  int width = 0; // display/recording window width
  int height = 0; // display/recording window height
  char set_window[20]; // display/recording window setting

  // activate software
  mj_activate("mjkey.txt");

  // load and compile model
  char error[1000] = "Could not load binary model";

  if( argc<2 ){
    m = mj_loadXML(xmlpath, 0, error, 1000);
  }
  else{
    if( strlen(argv[1])>4 && !strcmp(argv[1]+strlen(argv[1])-4, ".mjb") ){
      m = mj_loadModel(argv[1], 0);
    }
    else{
      m = mj_loadXML(argv[1], 0, error, 1000);
    }
  }
  if( !m ){
    mju_error_s("Load model error: %s", error);
  }

  // make data
  d = mj_makeData(m);
  // mj_forward(m, d); // if uncommented starts pend in upright pos for one frame

  // init GLFW
  if (!glfwInit()) {
    mju_error("Could not initialize GLFW");
  }

  // if display_window is true it will display the simulation window otherwise
  // it will not display the sim window and record the simulation
  if (display_window) {
    // create window, make OpenGL context current, request v-sync
    // GLFWwindow* window = glfwCreateWindow(1244, 700, "Demo", NULL, NULL);
    width = 1244;
    height = 700;
    strcpy(set_window, "Demo");
  }
  else {
    // create invisible window, single-buffered
    glfwWindowHint(GLFW_VISIBLE, 0);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
    // GLFWwindow* window = glfwCreateWindow(800, 800, "Invisible window", NULL, NULL);
    width = 800;
    height = 800;
    strcpy(set_window, "Invisible window");
  }

  GLFWwindow* window = glfwCreateWindow(width, height, set_window, NULL, NULL);
  glfwMakeContextCurrent(window);

  // initialize visualization data structures
  mjv_defaultCamera(&cam);
  mjv_defaultOption(&opt);
  mjv_defaultScene(&scn);
  mjr_defaultContext(&con);
  mjv_makeScene(m, &scn, 2000);                // space for 2000 objects
  mjr_makeContext(m, &con, mjFONTSCALE_200);   // model-specific context

  if (display_window){
    glfwSwapInterval(1);

    // install GLFW mouse and keyboard callbacks
    glfwSetKeyCallback(window, keyboard);
    glfwSetCursorPosCallback(window, mouse_move);
    glfwSetMouseButtonCallback(window, mouse_button);
    glfwSetScrollCallback(window, scroll);
  }

  if (video_length > simend){
    printf("video_length > simend!");
    video_length = simend;
  }

  // center and scale view
  double arr_view[] = {90.00, -56.00, 1.60, 0.00, 0.00, 0.00};
  cam.azimuth = arr_view[0];
  cam.elevation = arr_view[1];
  cam.distance = arr_view[2];
  cam.lookat[0] = arr_view[3];
  cam.lookat[1] = arr_view[4];
  cam.lookat[2] = arr_view[5];

  int t_size = 4;
  double camtime_arr[][2] = {{0,1},{1,5},{5,10},{10,20}};
  int t_final = camtime_arr[t_size-1][1];
  // printf("%d",t_final);

  int tspan[t_size][1];
  // int t_size = sizeof(*camtime_arr);
  // printf("t_size: %d\n",t_size);
  int t_total = 0;

  for (int i=0;i<t_size;i++){
    tspan[i][0] = (camtime_arr[i][1] - camtime_arr[i][0])*(10000/100);
    // printf("%d: %d\n",i,tspan[i][0]);
    t_total = t_total + tspan[i][0];
  }
  // printf("Make sure struct has this number: %d\n",t_total);

  double campos_arr[][2] = {{arr_view[0],arr_view[1]},{arr_view[0],arr_view[1]},{90 -90},{270 -90},{90,-56}};
  // double * camref_arr[t_total][2];
  // camref_arr = cam_ref_arr(camtime_arr, campos_arr, arr_view);
  int cam_size = 4; // num of pos - 1
  // printf("cam_size: %d\n",cam_size);
  double dcampos_yaw[cam_size][1];
  double dcampos_pitch[cam_size][1];

// problem with final dcampos -------*********
  for (int i=0;i<cam_size;i++){
    dcampos_yaw[i][0] = (campos_arr[i+1][0] - campos_arr[i][0]) / (tspan[i][0]);
    // printf("%f\n",dcampos_yaw[i][0]);
    dcampos_pitch[i][0] = (campos_arr[i+1][1] - campos_arr[i][1]) / (tspan[i][0]);
    // printf("%f\n",dcampos_pitch[i][0]);
  }

  int counter = 0;
  // double campos_traj[t_total][2] = {0};

  // Create struct of size t_total
  Data data[t_total];
  // campos_traj[0][0] = campos_arr[0][0];
  // data[0].cam_yaw = 90.00;//campos_arr[0][0];
  // campos_traj[0][1] = campos_arr[0][1];
  // data[0].cam_pitch = -56.0;//campos_arr[0][0];
  int t = 0;
  // printf("t_total: %d\n",t_total);
  // memcpy(&data[t].cam_yaw,&campos_arr[0][0],sizeof(float));
  // memcpy(&data[t].cam_pitch,&campos_arr[0][1],sizeof(float));
  data[t].cam_yaw = campos_arr[0][0];
  data[t].cam_pitch = campos_arr[0][1];
  // printf("%f\n",data[0].cam_yaw);
  // printf("%f\n",data[0].cam_pitch);
  // printf("*****\n");
  // printf("%f\n",dcampos_yaw[0][0]);
  // printf("%f\n",dcampos_yaw[1][0]);
  // printf("%f\n",dcampos_yaw[2][0]);
  //
  // printf("%f\n",dcampos_pitch[0][0]);
  // printf("%f\n",dcampos_pitch[1][0]);
  // printf("%f\n",dcampos_pitch[2][0]);

  // for (int i=0;i<t_size;i++){
  //   printf("%d: %d\n",i,tspan[i][0]);
  // }

  for (int j=0;j<t_size;j++){
    while (counter < tspan[j][0]){
      // campos_traj[t+1][0] = campos_traj[t][0] + dcampos_yaw[j];
      // campos_traj[t+1][1] = campos_traj[t][1] + dcampos_pitch[j];
      data[t+1].cam_yaw = data[t].cam_yaw + dcampos_yaw[j][0];
      // printf("%f\n",data[t].cam_yaw);
      data[t+1].cam_pitch = data[t].cam_pitch + dcampos_pitch[j][0];
      // printf("%f\n",data[t].cam_pitch);
      counter++;
      t++;
      // printf("%d\n",t);
    }
    counter = 0;
  }
  // printf("Done!\n");

  // for (int m=0;m<t_total;m++){
  //   printf("i: %d; yaw: %f; pitch: %f\n",m,data[m].cam_yaw,data[m].cam_pitch);
  //   printf("yaw: %f; pitch: %f",campos_traj[m][0],campos_traj[m][1]);
  // }
  if (record_data){
    init_save_data();
  }

  // initial conditions for pos & vel
  d->qpos[0] = 3.14/2 - 0.2;
  d->qvel[0] = 0;
  int count_campos = 0;
  int count_time = 0;

  if (display_window){
    // use the first while condition if you want to simulate for a period.
    while( !glfwWindowShouldClose(window))
    {
      // advance interactive simulation for 1/60 sec
      //  Assuming MuJoCo can simulate faster than real-time, which it usually can,
      //  this loop will finish on time for the next frame to be rendered at 60 fps.
      //  Otherwise add a cpu timer and exit this loop when it is time to render.
      // get framebuffer viewport
      mjtNum simstart = d->time;
      while( d->time - simstart < 1.0/60.0 ) //increasing from 60 to 240 decreases rendering speed
      {
      // Ensure that both the display & record conditions have the same code block
      // ***********************************************************************
          mj_step(m, d);
          // printf("%f\n",d->time);

          if (d->time - t_control > 1e-5)
          {
            loop(m,d);
            t_control = d->time;
          }

          if (d->time < t_final && count_time % 100 == 0){
            cam.azimuth = data[count_campos].cam_yaw;//campos_traj[count_campos][0];
            // printf("%f\n",data[count_campos].cam_yaw);
            cam.elevation = data[count_campos].cam_pitch;//campos_traj[count_campos][1];
            count_campos = count_campos + 1;
            // printf("%d\n",count_campos);
          }

          count_time = count_time + 1;
      }

      if (d->time>=simend)
      {
         fclose(fid);
         break;
       }
      // ***********************************************************************

      // get framebuffer viewport
      mjrRect viewport = {0, 0, 0, 0};
      glfwGetFramebufferSize(window, &viewport.width, &viewport.height);

      // update scene and render
      mjv_updateScene(m, d, &opt, NULL, &cam, mjCAT_ALL, &scn);
      mjr_render(viewport, &scn, &con);
      // printf("{%f, %f, %f, %f, %f, %f};\n",cam.azimuth,cam.elevation, cam.distance,cam.lookat[0],cam.lookat[1],cam.lookat[2]);

      // swap OpenGL buffers (blocking call due to v-sync)
      glfwSwapBuffers(window);

      // process pending GUI events, call GLFW callbacks
      glfwPollEvents();

    }
  }
  else{
    // set rendering to offscreen buffer
    mjr_setBuffer(mjFB_OFFSCREEN, &con);
    if (con.currentBuffer!=mjFB_OFFSCREEN) {
      printf("Warning: offscreen rendering not supported, using default/window framebuffer\n");
    }

    mjrRect viewport =  mjr_maxViewport(&con);
    int W = viewport.width;
    int H = viewport.height;

    // allocate rgb and depth buffers
    unsigned char* rgb = (unsigned char*) malloc(3*W*H);
    float* depth = (float*) malloc(sizeof(float)*W*H);
    if (!rgb || !depth) {
      mju_error("Could not allocate buffers");
    }

    // create output rgb file
    FILE* fp = fopen(rgb_file, "wb");
    if (!fp) {
      mju_error("Could not open rgbfile for writing");
    }

    // main loop
    frametime = 0;
    framecount = 0;

    while (d->time<t_final) {
      // render new frame if it is time (or first frame)
      if ((d->time-frametime)>1/fps || frametime==0) {

        // update scene & render in offscreen buffer
        mjv_updateScene(m, d, &opt, NULL, &cam, mjCAT_ALL, &scn);
        mjr_render(viewport, &scn, &con);

        // add time stamp in upper-left corner
        char stamp[50];
        sprintf(stamp, "Time = %.3f", d->time);
        mjr_overlay(mjFONT_NORMAL, mjGRID_TOPLEFT, viewport, stamp, NULL, &con);

        // read rgb and depth buffers
        mjr_readPixels(rgb, depth, viewport, &con);

        // write rgb image to file
        fwrite(rgb, 3, W*H, fp);

        // save simulation time
        frametime = d->time;
      }
      // Ensure that both the display & record conditions have the same code block
      // ***********************************************************************
      mj_step(m, d);
      // printf("%f\n",d->time);

      if (d->time - t_control > 1e-5)
      {
        loop(m,d);
        t_control = d->time;
      }

      if (d->time < t_final && count_time % 100 == 0){
        cam.azimuth = data[count_campos].cam_yaw;//campos_traj[count_campos][0];
        // printf("%f\n",data[count_campos].cam_yaw);
        cam.elevation = data[count_campos].cam_pitch;//campos_traj[count_campos][1];
        count_campos = count_campos + 1;
        // printf("%d\n",count_campos);
      }

      count_time = count_time + 1;

      if (d->time>=simend)
      {
        fclose(fid);
        break;
      }
      // ***********************************************************************
    }
    // close file, free buffers
    fclose(fp);
    free(rgb);
    free(depth);
  }

  // free visualization storage
  mjv_freeScene(&scn);
  mjr_freeContext(&con);

  // free MuJoCo model and data, deactivate
  mj_deleteData(d);
  mj_deleteModel(m);
  mj_deactivate();

  // terminate GLFW (crashes with Linux NVidia drivers)
  #if defined(__APPLE__) || defined(_WIN32)
      glfwTerminate();
  #endif

  return 1;
}
