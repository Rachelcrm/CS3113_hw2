#define GL_GLEXT_PROTOTYPES 1
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

// —— NEW STUFF —— //
#include <ctime>   //
#include "cmath"   //
// ——————————————— //

const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X      = 0,
          VIEWPORT_Y      = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char  V_SHADER_PATH[]          = "shaders/vertex_textured.glsl",
            F_SHADER_PATH[]          = "shaders/fragment_textured.glsl",
            PLAYER_SPRITE_FILEPATH[] = "/Users/rachelchen/Desktop/CS3113_hw2/butt1.png",
            PLAYER_SPRITE_FILEPATH2[] = "/Users/rachelchen/Desktop/CS3113_hw2/poop.png",
            PLAYER_SPRITE_FILEPATH3[] = "/Users/rachelchen/Desktop/CS3113_hw2/butt2.png",
            LEFT_PLAYER_WIN_FILEPATH[] = "/Users/rachelchen/Desktop/CS3113_hw2/left_win.png",
            RIGHT_PLAYER_WIN_FILEPATH[] = "/Users/rachelchen/Desktop/CS3113_hw2/right_win.png";

const float MILLISECONDS_IN_SECOND     = 1000.0;
const float MINIMUM_COLLISION_DISTANCE = 1.0f;

//whether the 1/2/3 ball is going left
bool ball_go_left = true,
     ball_go_down = true,
     second_ball_go_down = true,
     second_ball_go_left = true,
     third_ball_go_down = true,
     third_ball_go_left = true,
//any player wins
     player_left_win = false,
     player_right_win = false,

     transfer_to_one_player = false;

const int   NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL    = 0;
const GLint TEXTURE_BORDER     = 0;

SDL_Window* g_display_window;
bool  g_game_is_running = true;
float g_previous_ticks  = 0.0f;

ShaderProgram g_shader_program;
glm::mat4     g_view_matrix,
              g_projection_matrix,
              g_model_matrix, //right player board model matrix
              g_other_model_matrix,//left player/auto board model matrix
              g_ball_model_matrix,
              g_second_ball_model_matrix,
              g_third_ball_model_matrix,
              endgame_UI_model_matrix;

GLuint g_player_texture_id,
       g_other_texture_id,
       g_ball_texture_id,
       g_left_win_texture_id,
       g_right_win_texture_id;

//right player's board's position and movement
glm::vec3 g_player_position = glm::vec3(3.5f, 0.0f, 0.0f);
glm::vec3 g_player_movement = glm::vec3(0.0f, 0.0f, 0.0f);

//left player's board's position and movement
glm::vec3 g_other_position  = glm::vec3(-3.0f, 0.0f, 0.0f);
glm::vec3 g_other_movement  = glm::vec3(1.0f, 1.0f, 0.0f);

glm::vec3 g_ball_position  = glm::vec3(3.0f, 0.0f, 0.0f);
glm::vec3 g_ball_movement  = glm::vec3(1.0f, 1.0f, 0.0f);

glm::vec3 g_second_ball_position  = glm::vec3(3.0f, 0.0f, 0.0f);
glm::vec3 g_second_movement  = glm::vec3(1.0f, 1.0f, 0.0f);

glm::vec3 g_third_ball_position  = glm::vec3(2.0f, 0.0f, 0.0f);
glm::vec3 g_third_movement  = glm::vec3(1.0f, 1.0f, 0.0f);

glm::vec3 ball_scale = glm::vec3(0.3f, 0.3f, 0.0f);
glm::vec3 board_scale = glm::vec3(1.0f, 1.4f, 0.0f);
glm::vec3 endgame_UI_scale = glm::vec3(12.0f, 3.0f, 0.0f);


const float g_player_speed = 4.0f;
const float g_other_speed = 4.0f;

// boundaries of the screen
const float left_wall_x = -5.0f,
            right_wall_x = 5.0f,
            up_wall_y = 4.0f,
            down_wall_y = -3.35f;

bool endgame = false;

// Player can choose how many balls they want, from 1 to 3
int custom_ball_num = 1;

GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    
    return textureID;
}


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Hello, Collisions!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    //initialize the right board
    g_model_matrix     = glm::mat4(1.0f);
    g_model_matrix     = glm::translate(g_model_matrix, g_player_position);
    g_model_matrix = glm::scale(g_model_matrix, board_scale);
    
    //initialize the left board
    g_other_model_matrix = glm::mat4(1.0f);
    g_other_model_matrix     = glm::translate(g_other_model_matrix, g_other_position);
    g_other_model_matrix = glm::scale(g_other_model_matrix, board_scale);
    
    g_ball_model_matrix = glm::mat4(1.0f);
    g_ball_model_matrix = glm::translate(g_ball_model_matrix, g_ball_position);
    
    g_second_ball_model_matrix = glm::mat4(1.0f);
    g_second_ball_model_matrix = glm::translate(g_ball_model_matrix, g_ball_position);
    
    g_third_ball_model_matrix = glm::mat4(1.0f);
    g_third_ball_model_matrix = glm::translate(g_ball_model_matrix, g_ball_position);
    
    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    endgame_UI_model_matrix = glm::mat4(1.0f);
    endgame_UI_model_matrix = glm::translate(endgame_UI_model_matrix, glm::vec3(3.8f, 4.0f, 0.0f));
    endgame_UI_model_matrix = glm::scale(endgame_UI_model_matrix, endgame_UI_scale);
    
    g_player_texture_id = load_texture(PLAYER_SPRITE_FILEPATH);
    g_other_texture_id  = load_texture(PLAYER_SPRITE_FILEPATH3);
    g_ball_texture_id = load_texture(PLAYER_SPRITE_FILEPATH2);
    g_left_win_texture_id = load_texture(LEFT_PLAYER_WIN_FILEPATH);
    g_right_win_texture_id = load_texture(RIGHT_PLAYER_WIN_FILEPATH);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void process_input()
{
    g_player_movement = glm::vec3(0.0f);
    g_other_movement = glm::vec3(0.0f);
    g_ball_movement = glm::vec3(0.0f);
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_q:
                        g_game_is_running = false;
                        break;
                        
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    // control the right board and make sure it can't go higher/lower than the screen
    if (key_state[SDL_SCANCODE_UP] && g_player_position.y + board_scale.y / 2 < up_wall_y)
    {
        g_player_movement.y = 20.0f;
    }
    else if (key_state[SDL_SCANCODE_DOWN] && g_player_position.y> down_wall_y)
    {
        g_player_movement.y = -20.0f;
    }
    
    // switch to one-player-mode
    if (key_state[SDL_SCANCODE_T]) {
        transfer_to_one_player = true;
    }
    
    if (glm::length(g_player_movement) > 1.0f)
    {
        g_player_movement = glm::normalize(g_player_movement);
    }
    
    // control the left board and make sure it can't go higher/lower than the screen
    if (key_state[SDL_SCANCODE_W] && g_other_position.y + board_scale.y / 2 < up_wall_y)
    {
        g_other_movement.y = 20.0f;
    }
    else if (key_state[SDL_SCANCODE_S] && g_other_position.y > down_wall_y)
    {
        g_other_movement.y = -20.0f;
    }
    
    if (glm::length(g_other_movement) > 1.0f)
    {
        g_other_movement = glm::normalize(g_other_movement);
    }
    
    // set number of balls
    if (key_state[SDL_SCANCODE_1]) {custom_ball_num = 1;}
    if (key_state[SDL_SCANCODE_2]) {custom_ball_num = 2;}
    if (key_state[SDL_SCANCODE_3]) {custom_ball_num = 3;}
}


// control each ball's movement and collision
void each_ball_collision(glm::vec3& ball_position, bool& ball_down, bool& ball_left, glm::vec3& ball_movement,
                         glm::mat4& model_matrix, float delta_time) {
    model_matrix = glm::mat4(1.0f);
    float d_to_up_w = up_wall_y - ball_position.y;
    float d_to_down_w = down_wall_y - ball_position.y;
    if (d_to_down_w >= 0.0f) {
        ball_position  = glm::vec3(ball_position.x, -3.34f, 0.0f);
        ball_down = !ball_down;
    }
    else if (d_to_up_w <= 0.0f) {
        ball_position  = glm::vec3(ball_position.x, 3.99f, 0.0f);
        ball_down = !ball_down;
    }
    
    //box-to-box collision for left board
    float x_d_to_left_b = fabs(ball_position.x-g_other_position.x)-((0.3f+0.2f)/2.0f); //width of board+ball
    float y_d_to_left_b = fabs(ball_position.y-(g_other_position.y+0.2))-((0.2f+1.4f)/2.0f); //height of board+ball
    if (x_d_to_left_b <= 0 && y_d_to_left_b <= 0) {
        ball_position  = glm::vec3(-2.7f, ball_position.y, 0.0f);
        ball_left = !ball_left;
    }
    
    //box-to-box collision for right board
    float x_d_to_right_b = fabs((ball_position.x)-g_player_position.x)-((0.3+0.2f)/2.0f);
    float y_d_to_right_b = fabs(ball_position.y-(g_player_position.y+0.2))-((0.2f+1.6f)/2.0f);
    if (x_d_to_right_b <= 0 && y_d_to_right_b <= 0) {
        ball_position  = glm::vec3(3.2f, ball_position.y, 0.0f);
        ball_left = !ball_left;
    }
    
    if (ball_down) {
        ball_movement.y = -2.0f;}
    else {
        ball_movement.y = 2.0f;}
    if (ball_left) {
        ball_movement.x = -2.0f;}
    else {ball_movement.x = 2.0f;}
    
    ball_position += ball_movement * 1.0f * delta_time;
    model_matrix = glm::translate(model_matrix, ball_position);
    
    if (transfer_to_one_player) {
        //Make the right auto board move based on the y-coordinate of the nearest ball so that one player can keep playing
        float nearest_ball_x_position;
        if (custom_ball_num == 1) {nearest_ball_x_position = g_ball_position.x;}
        else if (custom_ball_num == 2) {nearest_ball_x_position = std::max(g_ball_position.x, g_second_ball_position.x);}
        else if (custom_ball_num == 3) {nearest_ball_x_position = std::max(g_ball_position.x, std::max(g_second_ball_position.x, g_third_ball_position.x));}
        float right_board_y;
        if (nearest_ball_x_position == g_ball_position.x) {right_board_y = g_ball_position.y;}
        else if (nearest_ball_x_position == g_second_ball_position.x) {right_board_y = g_second_ball_position.y;}
        else if (nearest_ball_x_position == g_third_ball_position.x) {right_board_y = g_third_ball_position.y;}
        g_player_position = glm::vec3(3.5f, right_board_y, 0.0f);
    }
    //"else" is outside of the function
}


void update()
{
    if (!endgame){
        float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
        float delta_time = ticks - g_previous_ticks;
        g_previous_ticks = ticks;
        
        //update the right board
        g_model_matrix     = glm::mat4(1.0f);
        g_model_matrix     = glm::translate(g_model_matrix, g_player_position);
        g_model_matrix = glm::scale(g_model_matrix, board_scale);
        
        //update the left board
        g_other_model_matrix = glm::mat4(1.0f);
        g_other_position += g_other_movement * g_other_speed * delta_time;
        g_other_model_matrix     = glm::translate(g_other_model_matrix, g_other_position);
        g_other_model_matrix = glm::scale(g_other_model_matrix, board_scale);
        
        //set the ball matrix
        g_ball_model_matrix = glm::mat4(1.0f);
        if (custom_ball_num == 3) {
            each_ball_collision(g_third_ball_position, third_ball_go_down, third_ball_go_left, g_third_movement, g_third_ball_model_matrix, delta_time);
            each_ball_collision(g_second_ball_position, second_ball_go_down, second_ball_go_left, g_second_movement, g_second_ball_model_matrix, delta_time);
            each_ball_collision(g_ball_position, ball_go_down, ball_go_left, g_ball_movement, g_ball_model_matrix, delta_time);
            if (!transfer_to_one_player) {g_player_position += g_player_movement * g_player_speed * delta_time;}
        }
        
        else if (custom_ball_num == 2){
            each_ball_collision(g_second_ball_position, second_ball_go_down, second_ball_go_left, g_second_movement, g_second_ball_model_matrix, delta_time);
            each_ball_collision(g_ball_position, ball_go_down, ball_go_left, g_ball_movement, g_ball_model_matrix, delta_time);
            if (!transfer_to_one_player) {g_player_position += g_player_movement * g_player_speed * delta_time;}
        }
        
        else if (custom_ball_num == 1) {
            each_ball_collision(g_ball_position, ball_go_down, ball_go_left, g_ball_movement, g_ball_model_matrix, delta_time);
            if (!transfer_to_one_player) {g_player_position += g_player_movement * g_player_speed * delta_time;}
            }
        
        //Check whether any of the player wins
        bool pass_right = (right_wall_x-g_ball_position.x <= 0 || right_wall_x-g_second_ball_position.x <= 0 || right_wall_x-g_third_ball_position.x <= 0);
        bool pass_left = (g_ball_position.x-left_wall_x <= 0 || g_second_ball_position.x - left_wall_x <= 0 ||
                          g_third_ball_position.x-left_wall_x <= 0);
        
        if (pass_left == true) {
            player_right_win = true;
            endgame = true;
        }
        if (pass_right == true) {
            player_left_win = true;
            endgame = true;
        }
    }
}


void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    float vertices[] = {
        -0.5f, -0.5f, -0.2f, -0.5f, -0.2f, 0.5f,  // rectangle 1&2
        -0.5f, -0.5f, -0.2f, 0.5f, -0.5f, 0.5f
    };
    
    float ball_vertices[] = {
        -0.5f, -0.5f, -0.1f, -0.5f, -0.1f, -0.1f,  // ball
        -0.5f, -0.5f, -0.1f, -0.1f, -0.5f, -0.1f
    };

    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    draw_object(g_model_matrix, g_player_texture_id);
    draw_object(g_other_model_matrix, g_other_texture_id);
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, ball_vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    //draw balls based on number of balls
    draw_object(g_ball_model_matrix, g_ball_texture_id);
    if (custom_ball_num == 2){
        draw_object(g_second_ball_model_matrix, g_ball_texture_id);
    }
    if (custom_ball_num == 3){
        draw_object(g_second_ball_model_matrix, g_ball_texture_id);
        draw_object(g_third_ball_model_matrix, g_ball_texture_id);
    }
    
    //endgame UI message
    if (player_left_win) {
        draw_object(endgame_UI_model_matrix, g_left_win_texture_id);
    }
    else if (player_right_win) {
        draw_object(endgame_UI_model_matrix, g_right_win_texture_id);}
    
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(g_display_window);
}


void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();
    
    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
