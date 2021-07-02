#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Para usar strings

#ifdef WIN32
#include <windows.h> // Apenas para Windows
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>   // Funções da OpenGL
#include <GL/glu.h>  // Funções da GLU
#include <GL/glut.h> // Funções da FreeGLUT
#endif

// SOIL é a biblioteca para leitura das imagens
#include <SOIL.h>

// Um pixel RGB (24 bits)
typedef struct
{
    unsigned char r, g, b;
} RGB8;

// Uma imagem RGB
typedef struct
{
    int width, height;
    RGB8 *img;
} Img;

// Protótipos
void load(char *name, Img *pic);
void uploadTexture();
void seamcarve(int targetWidth); // executa o algoritmo
void freemem();                  // limpa memória (caso tenha alocado dinamicamente)

// Funções da interface gráfica e OpenGL
void init();
void draw();
void keyboard(unsigned char key, int x, int y);
void arrow_keys(int a_keys, int x, int y);

// Largura e altura da janela
int width, height;

// Largura desejada (selecionável)
int targetW;

// Identificadores de textura
GLuint tex[3];

// As 3 imagens
Img pic[3];
Img *source;
Img *mask;
Img *target;

// Imagem selecionada (0,1,2)
int sel;

// Carrega uma imagem para a struct Img
void load(char *name, Img *pic)
{
    int chan;
    pic->img = (RGB8 *)SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
    if (!pic->img)
    {
        printf("SOIL loading error: '%s'\n", SOIL_last_result());
        exit(1);
    }
    printf("Load: %d x %d x %d\n", pic->width, pic->height, chan);
}

//
// Implemente AQUI o seu algoritmo
void seamcarve(int targetWidth)
{
    // Aplica o algoritmo e gera a saida em target->img...

    RGB8(*ptr)
    [target->width] = (RGB8(*)[target->width])target->img;

    RGB8(*ptr4)
    [source->width] = (RGB8(*)[source->width])source->img;


    for (int xtotal = 0; xtotal < target->height; xtotal++)
    {
        for (int ytotal = 0; ytotal < targetW; ytotal++)
            ptr[xtotal][ytotal] = ptr4[xtotal][ytotal];

        for (int ytotal = targetW; ytotal < target->width; ytotal++)
            ptr[xtotal][ytotal].r = ptr[xtotal][ytotal].g = ptr[xtotal][ytotal].b = 255;
    }

    //Calculo da energia de cada pixel
    int matrizEnergia[targetWidth][target->height];

    RGB8(*ptr2)
    [targetWidth] = (RGB8(*)[targetWidth])target->img;

    for (int x = 0; x < targetWidth; x++){
        for (int y = 0; y < target->height; y++){
            int Xmais1, Xmenos1, Ymais1, Ymenos1;
            if(x==0){Xmenos1 = targetWidth-1; Xmais1=x+1;} else if(x==targetWidth-1){Xmais1 = 0;Xmenos1=x-1;} else {Xmais1=x+1; Xmenos1=x-1;} //source->width-1
            if(y==0){Ymenos1 = target->height-1; Ymais1=y+1;}else if(y==target->height-1){Ymais1 = 0; Ymenos1=y-1;} else {Ymenos1=y-1; Ymais1=y+1;} //source->height-1

            int deltaRx, deltaGx, deltaBx;
            deltaRx = ptr2[Xmais1][y].r - ptr2[Xmenos1][y].r;
            deltaGx = ptr2[Xmais1][y].g - ptr2[Xmenos1][y].g;
            deltaBx = ptr2[Xmais1][y].b - ptr2[Xmenos1][y].b;

            int deltaX = pow(deltaRx, 2) + pow(deltaGx, 2) + pow(deltaBx, 2);

            int deltaRy, deltaGy, deltaBy;
            deltaRy = ptr2[x][Ymais1].r - ptr2[x][Ymenos1].r;
            deltaGy = ptr2[x][Ymais1].g - ptr2[x][Ymenos1].g;
            deltaBy = ptr2[x][Ymais1].b - ptr2[x][Ymenos1].b;

            int deltaY = pow(deltaRy, 2) + pow(deltaGy, 2) + pow(deltaBy, 2);

            matrizEnergia[x][y] = deltaX + deltaY;   
        } 
    }

    //mascara
    RGB8(*ptr3)
    [mask->width] = (RGB8(*)[mask->width])mask->img; 
    for(int g=0; g<mask->height; g++){
        for(int h=0; h<mask->width; h++){
            if(ptr3[g][h].b < 100 && ptr3[g][h].g < 100){
                matrizEnergia[g][h] = -10000;
            } else if (ptr3[g][h].r < 100 && ptr3[g][h].b < 100){
                matrizEnergia[g][h] *=10000;
            }
        }
    }
    

    //matriz de custo acumulado
    int matrizCustoAcumulado[targetWidth][target->height];
    for (int y=0; y<target->height; y++){
        matrizCustoAcumulado[0][y] = matrizEnergia[0][y];
    }

    for (int x=1; x<targetWidth; x++){ 
        for (int y=0; y<target->height; y++){
            int menor = matrizCustoAcumulado[x-1][y]; //assume o logo de cima como menor para comparar depois
            if (y==0) //comparar só com a da diagonal direita
            {
                if (matrizCustoAcumulado[x-1][y+1] < menor)
                    menor = matrizCustoAcumulado[x-1][y+1];
            } else if (y==targetWidth-1) //comparar só com o da diagonal esquerda 
            {
               if (matrizCustoAcumulado[x-1][y-1] < menor)
                    menor = matrizCustoAcumulado[x-1][y-1];
            } else { //compara as duas diagonais
                if (matrizCustoAcumulado[x-1][y+1] < menor)
                    menor = matrizCustoAcumulado[x-1][y+1];
                if (matrizCustoAcumulado[x-1][y-1] < menor)
                    menor = matrizCustoAcumulado[x-1][y-1];    
            }

            matrizCustoAcumulado[x][y] = matrizEnergia[x][y] + menor;
        }
    }

    //Identificação do melhor caminho
    int pixelsRemover[target->height];
    int contadorRemover = 1;
    int menorSomaAcumulada = matrizCustoAcumulado[target->height-1][0]; //assume o primeiro indice da ultima linha para comparar depois

    for (int y=0; y<targetWidth; y++){
        if (matrizCustoAcumulado[target->height-1][y] < menorSomaAcumulada){ 
            menorSomaAcumulada = matrizCustoAcumulado[target->height-1][y];
            pixelsRemover[0] = y; //armazena o indice da coluna
        }
    }

    int wid = pixelsRemover[0];
    for (int x=target->height-2; x>=0; x--){//inicia no menor valor da ultima linha e vai comparando os de cima
        int menor = matrizCustoAcumulado[x][wid];
        if (wid==0){
            if (matrizCustoAcumulado[x][wid+1] < menor){
                pixelsRemover[contadorRemover] = wid+1;
                wid++;
                contadorRemover++;
            } else {
                pixelsRemover[contadorRemover] = wid;
                contadorRemover++;
            }
        } else if (wid==targetWidth-1){ 
            if (matrizCustoAcumulado[x][wid-1] < menor){
                pixelsRemover[contadorRemover] = wid-1;
                wid--;
                contadorRemover++;
            } else {
                pixelsRemover[contadorRemover] = wid;
                contadorRemover++;
            }
        } else {
            if (matrizCustoAcumulado[x][wid+1] < menor){
                pixelsRemover[contadorRemover] = wid+1;
                wid++;
                contadorRemover++;
            } else if (matrizCustoAcumulado[x][wid-1] < menor){
                pixelsRemover[contadorRemover] = wid-1;
                wid--;
                contadorRemover++;
            }else {
                pixelsRemover[contadorRemover] = wid;
                contadorRemover++;
            }
        } 
    }

    //Remoção do seam com melho3r caminho
    int y2 = 0, x2 = 0;
    for (int xtotal=0, aux=target->height-1; xtotal<target->height; xtotal++, aux--){
        y2=0;
        for (int ytotal=0; ytotal<targetWidth; ytotal++){            
            if(ytotal!=pixelsRemover[aux]){
                ptr[xtotal][ytotal] = ptr4[x2][y2];
                y2++;
            }
        }
        x2++;
        for (int ytotal = targetWidth; ytotal<target->width; ytotal++){
            ptr[xtotal][ytotal].r = ptr[xtotal][ytotal].g = ptr[xtotal][ytotal].b = 0;
        }            
    }

    // Chame uploadTexture a cada vez que mudar
    // a imagem (pic[2])
    uploadTexture();
    glutPostRedisplay();
}

void freemem()
{
    // Libera a memória ocupada pelas 3 imagens
    free(pic[0].img);
    free(pic[1].img);
    free(pic[2].img);
}

/********************************************************************
 * 
 *  VOCÊ NÃO DEVE ALTERAR NADA NO PROGRAMA A PARTIR DESTE PONTO!
 *
 ********************************************************************/
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("seamcarving [origem] [mascara]\n");
        printf("Origem é a imagem original, mascara é a máscara desejada\n");
        exit(1);
    }
    glutInit(&argc, argv);

    // Define do modo de operacao da GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // pic[0] -> imagem original
    // pic[1] -> máscara desejada
    // pic[2] -> resultado do algoritmo

    // Carrega as duas imagens
    load(argv[1], &pic[0]);
    load(argv[2], &pic[1]);

    if (pic[0].width != pic[1].width || pic[0].height != pic[1].height)
    {
        printf("Imagem e máscara com dimensões diferentes!\n");
        exit(1);
    }

    // A largura e altura da janela são calculadas de acordo com a maior
    // dimensão de cada imagem
    width = pic[0].width;
    height = pic[0].height;

    // A largura e altura da imagem de saída são iguais às da imagem original (1)
    pic[2].width = pic[1].width;
    pic[2].height = pic[1].height;

    // Ponteiros para as structs das imagens, para facilitar
    source = &pic[0];
    mask = &pic[1];
    target = &pic[2];

    // Largura desejada inicialmente é a largura da janela
    targetW = target->width;

    // Especifica o tamanho inicial em pixels da janela GLUT
    glutInitWindowSize(width, height);

    // Cria a janela passando como argumento o titulo da mesma
    glutCreateWindow("Seam Carving");

    // Registra a funcao callback de redesenho da janela de visualizacao
    glutDisplayFunc(draw);

    // Registra a funcao callback para tratamento das teclas ASCII
    glutKeyboardFunc(keyboard);

    // Registra a funcao callback para tratamento das setas
    glutSpecialFunc(arrow_keys);

    // Cria texturas em memória a partir dos pixels das imagens
    tex[0] = SOIL_create_OGL_texture((unsigned char *)pic[0].img, pic[0].width, pic[0].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    tex[1] = SOIL_create_OGL_texture((unsigned char *)pic[1].img, pic[1].width, pic[1].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Exibe as dimensões na tela, para conferência
    printf("Origem  : %s %d x %d\n", argv[1], pic[0].width, pic[0].height);
    printf("Máscara : %s %d x %d\n", argv[2], pic[1].width, pic[0].height);
    sel = 0; // pic1

    // Define a janela de visualizacao 2D
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, width, height, 0.0);
    glMatrixMode(GL_MODELVIEW);

    // Aloca memória para a imagem de saída
    pic[2].img = malloc(pic[1].width * pic[1].height * 3); // W x H x 3 bytes (RGB)
    // Pinta a imagem resultante de preto!
    memset(pic[2].img, 0, width * height * 3);

    // Cria textura para a imagem de saída
    tex[2] = SOIL_create_OGL_texture((unsigned char *)pic[2].img, pic[2].width, pic[2].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Entra no loop de eventos, não retorna
    glutMainLoop();
}

// Gerencia eventos de teclado
void keyboard(unsigned char key, int x, int y)
{
    if (key == 27)
    {
        // ESC: libera memória e finaliza
        freemem();
        exit(1);
    }
    if (key >= '1' && key <= '3')
        // 1-3: seleciona a imagem correspondente (origem, máscara e resultado)
        sel = key - '1';
    if (key == 's')
    {
        seamcarve(targetW);
    }
    glutPostRedisplay();
}

void arrow_keys(int a_keys, int x, int y)
{
    switch (a_keys)
    {
    case GLUT_KEY_RIGHT:
        if (targetW <= pic[2].width - 10)
            targetW += 10;
        seamcarve(targetW);
        break;
    case GLUT_KEY_LEFT:
        if (targetW > 10)
            targetW -= 10;
        seamcarve(targetW);
        break;
    default:
        break;
    }
}
// Faz upload da imagem para a textura,
// de forma a exibi-la na tela
void uploadTexture()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 target->width, target->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, target->img);
    glDisable(GL_TEXTURE_2D);
}

// Callback de redesenho da tela
void draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Preto
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Para outras cores, veja exemplos em /etc/X11/rgb.txt

    glColor3ub(255, 255, 255); // branco

    // Ativa a textura corresponde à imagem desejada
    glBindTexture(GL_TEXTURE_2D, tex[sel]);
    // E desenha um retângulo que ocupa toda a tela
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2f(0, 0);

    glTexCoord2f(1, 0);
    glVertex2f(pic[sel].width, 0);

    glTexCoord2f(1, 1);
    glVertex2f(pic[sel].width, pic[sel].height);

    glTexCoord2f(0, 1);
    glVertex2f(0, pic[sel].height);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Exibe a imagem
    glutSwapBuffers();
}
