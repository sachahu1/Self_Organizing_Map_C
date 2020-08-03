#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#include <stdarg.h>
#include <errno.h>
#include <termios.h> //non standard
#include <unistd.h> //non standard
#include <dirent.h> //non standard

#if defined(_WIN32)
    #define makeDir(folder) _mkdir(folder);
#else 
    #include <sys/stat.h> // for windows use _mkdir
    #define makeDir(folder) mkdir(folder, 0777); 
    #endif

#define KRED  "\x1B[41m"
#define KGRN  "\x1B[42m"
#define KYEL  "\x1B[44m"
#define KNRM  "\x1B[0m"


struct irisDouble {
    double* v;
    char* name;
    double norme;
};
typedef struct irisDouble iris;

struct coordonnees {
    int i;
    int j;
};
typedef struct coordonnees coord;

struct Neuron {
    double* v;
    char* name;
    char label;
};
typedef struct Neuron neuron;

struct NeuralNetworkMap {
    neuron** N;
    coord dimension;
    int dimVect;
};
typedef struct NeuralNetworkMap Map;

struct configurationFile {
    int dimVect;
    int NbVect;
    coord dimensionTableau;
    double mini;
    double maxi;
    double alphaInit1;
    double alphaInit2;
    int phase1;
    int phase2;
    int rayon1;
    int rayon2;
};
typedef struct configurationFile Configurations;

int menu ( int argc, char* Args[])
{
    static struct termios oldMask, newMask;
    char **table  = NULL;

    int i = 0;
    int choix = -1;
    int position = 0;


    tcgetattr ( STDIN_FILENO, &oldMask );
    newMask = oldMask;
    newMask.c_lflag &= ~(ICANON); // avoid <enter>
    newMask.c_lflag &= ~(ECHO); // hide text typed

    tcsetattr( STDIN_FILENO, TCSANOW, &newMask );

    table = malloc ( sizeof ( char * ) * argc );

    for ( i = 0; i < argc; i++ ){
        table[ i ] = Args[i];
    }

    do{
        for ( i = 0; i < argc; i++ ){
            printf ( " %c %s\n", ( position == i )?'>':' ', table[ i ] );
        }
        switch ( i = getchar ( ) ){
            case 0x41:
            // up key is : 0x1b5b41
                position = ( position - 1 + argc ) % argc;
                break;
            case 0x42:
            // down key is : 0x1b5b42
                position = ( position + 1 ) % argc;
                break;
            case '\n':
                choix = position;
                break;
        }
        if ( choix < 0 ){
            printf ( "\x1B[%dA", argc );
        }
    }
    while ( choix < 0 );
    free ( table );
    tcsetattr( STDIN_FILENO, TCSANOW, &oldMask );
    return ( choix );
}

double parseConfig(char *line)
{
    char *cfline;
    char *buf = (char*)malloc(strlen(line)+1);
    double value;
    strcpy(buf, line);

    cfline = strstr((char*)buf,"=");
    cfline = cfline + strlen("=");
    sscanf(cfline, "%lf", &value);
   
    free(buf);
    return value;
}

void readConfig(char *path, Configurations* config) {
    FILE* fp;
    char buf[256];
    int Nbparam = 12;
    double* param;
    char *beg;
    coord dim;
    param = malloc(Nbparam * sizeof(double));
    
    if ((fp=fopen(path,"r")) != NULL){
        int i = 0;
        while (fgets(buf, sizeof(buf), fp) != NULL){
            char *end = buf +strlen(buf) -1;
            if (*end == '\n')
                *end = '\0';
            beg = buf;
            while (*beg == ' ')
                ++beg;

            if (*beg == '#')
                if (fgets(buf,sizeof(buf),fp)==NULL)
            break;

            param[i] = parseConfig(buf);
            ++i;
        }
        config->dimVect = param[0];
        config->NbVect = param[1];
        dim.i = param[2];
        dim.j = param[3];
        config->dimensionTableau = dim;
        config->mini = param[4];
        config->maxi = param[5];
        config->alphaInit1 = param[6];
        config->alphaInit2 = param[7];
        config->phase1 = param[8];
        config->phase2 = param[9];
        config->rayon1 = param[10];
        config->rayon2 = param[11];
    }
    else
        perror("fopen source-file");
    fclose(fp);
    free(param);
}


int countLines(char *path) {
    int j = 0;
    FILE* fp;
    char buf[256];
    if ((fp=fopen(path,"r")) != NULL)
        while (fgets(buf, sizeof(buf), fp) != NULL)
            ++j;
    else
        perror("fopen source-file");
    fclose(fp);

    return j;
}

void parseLine(char *line, iris* irisnum, int dimVect)
{
    char *token;
    char name[32];
    int j = 0;
    char *buf = (char*)malloc(strlen(line)+1);
    strcpy(buf, line);
    for (token = strtok(buf,","); token != NULL; token = strtok(NULL,",")){
            
        if (j < dimVect)
            sscanf(token, "%lf", &irisnum->v[j]);
        else if (j == dimVect)
        {
            sscanf(token, "%s", name);
            irisnum->name = (char*)malloc(strlen(name) + 1);
            strcpy(irisnum->name, name);
        }
        else
            perror("unknown token");
        ++j;
    }
    free(buf);
}

void parseFile(char *path, int nbLines, iris *DataBase, int dimVect) {
    FILE* fp;
    char buf[256];
    if ((fp = fopen(path,"r")) != NULL){
        int j = 0;
        while (j < nbLines && fgets(buf, sizeof(buf), fp) != NULL){
            char *end = buf +strlen(buf) -1;
            if (*end == '\n')
                *end = '\0';
            parseLine(buf, &DataBase[j], dimVect);
            ++j;
        }
    }
    else
        perror("fopen source-file");
    fclose(fp);
}



void parseReportConfiguration(char* reportName, Configurations* config){
    FILE* fp;
    int i;
    char buf[256];
    char* token;
    char *end;

    if ((fp =fopen(reportName,"r")) != NULL){
        // Skip first lines
        for (i = 0; i < 3; ++i){
            if(fgets(buf, sizeof(buf), fp)==NULL){
                printf("Incorect file format");
                return;
            }
        }

        // acquire dimVect
        if(fgets(buf,sizeof(buf),fp) == NULL){
            printf("Incorect file format\n");
            return;
        }
        char *end = buf + strlen(buf) -1;
        if (*end == '\n')
            *end = '\0';
        token = strstr(buf,":");
        token = token + strlen(": ");
        sscanf(token, "%d", &config->dimVect);

        // acquire Neurons
        if(fgets(buf, sizeof(buf), fp) == NULL){
            printf("Incorect file format\n");
            return;
        }
        *end = buf +strlen(buf) -1;
        if (*end == '\n')
            *end = '\0';
        token = strstr((char*)buf,":");
        token = token + strlen(":");
        sscanf(token, "%d", &config->NbVect);

        // acquire dimensions
        if(fgets(buf, sizeof(buf), fp) == NULL){
            printf("Incorect file format\n");
            return;
        }
        *end = buf +strlen(buf) -1;
        if (*end == '\n')
            *end = '\0';
        token = strstr((char*)buf,":");
        token = token + strlen(": ");
        strcpy(buf,token);
        token = strtok(token, "by");
        sscanf(token,"%d",&config->dimensionTableau.i);
        token = strstr((char*)buf, "by");
        token = token + strlen("by ");
        sscanf(token,"%d", &config->dimensionTableau.j);
        
        // acquire range
        if(fgets(buf, sizeof(buf), fp) == NULL){
            printf("Incorect file format\n");
            return;
        }
        //*end marche pas ici
        
        token = strstr((char*)buf,"between");
        token = token + strlen("between ");
        strcpy(buf,token);
        token = strtok(token, "and");
        sscanf(token,"%lf",&config->mini);
        token = strstr((char*)buf, "and");
        token = token + strlen("and ");
        sscanf(token,"%lf", &config->maxi);

        // acquire phase 1
        /*
        fgets(buf, sizeof(buf), fp);
        token = strstr((char*)buf, "has ");
        token = token + strlen(" has");
        printf("%s %s\n",buf, token);
        strcpy(buf, token); //strcpy mange des caracteres
        printf("%s %s\n",buf, token);*/

        fclose(fp);
    }
    else
        printf("The error is - %d\n", errno);
}
void parseReportMap(char* reportName, Map* NNmap){
    FILE* fp;
    int i,j, k;
    char buf[256];
    char* end;
    char* token;
    char* subtoken;
    char c;

    if ((fp =fopen(reportName,"r")) != NULL){
        // Skip first lines
        for (i = 0; i < 16; ++i){
            if(fgets(buf, sizeof(buf), fp) == NULL){
                printf("Incorect file format");
                return;
            }
        }

        //parse labels
        for(i = 0;i < NNmap->dimension.i; ++i){
            for(j = 0; j < NNmap->dimension.j; ++j){
                c = fgetc(fp);
                if (c =='A' || c== 'B' || c=='C'){
                NNmap->N[i][j].label = c;
                fgetc(fp);
                }
            }
            fgetc(fp);    
        }
        if(fgets(buf, sizeof(buf), fp) == NULL){
            printf("Incorect file format");
            return;
        }
        if(fgets(buf, sizeof(buf), fp) == NULL){
            printf("Incorect file format");
            return;
        }
        if(fgets(buf, sizeof(buf), fp) == NULL){
            printf("Incorect file format");
            return;
        }

/*
        for(i=0; i < NNmap->dimension.i; ++i){
            fgets(buf, sizeof(buf),fp);
            char *end = buf + strlen(buf) - 1;
            if (*end == '\n')
                *end = '\0';
            j = 0;
            //printf("%s\n",buf );
                for (token = strtok(buf, ";"); token != NULL; token = strtok(NULL, ";")){
                    k = 0;
                    ++j;
                    printf("%s\n %s\n",buf, token );
                    for (subtoken = strtok(token, " "); subtoken != NULL; subtoken = strtok(NULL, " ")){
                        sscanf(subtoken,"%f",&NNmap->N[i][j].v[k]);
                        ++k;
                        printf("%s %s %s\n",buf, token, subtoken );
                    }
                }
        }
        */

        fclose(fp);
    }

    else
        printf("The error is - %d\n", errno);
}

void writeReport (char* reportName, Map NNmap, Configurations config, double percentage){
    int i,j,k;
    FILE* Report = NULL;
    Report = fopen(reportName, "w+");
    fputs("---------------- Report File ----------------\n\n", Report);
    fputs("Launching Configurations : \n", Report);
    fprintf(Report, "Vector Dimension : %d\n", config.dimVect);
    fprintf(Report, "Number of Neurons : %d\n", config.NbVect);
    fprintf(Report, "Map Size : %d by %d\n", config.dimensionTableau.i, config.dimensionTableau.j);
    fprintf(Report, "Neurons generated between %f and %f\n", config.mini, config.maxi);
    fprintf(Report, "First phase has %d iterations with neighborhood %d with alpha : %f\n",config.phase1, config.rayon1, config.alphaInit1);
    fprintf(Report, "First phase has %d iterations with neighborhood %d alpha : %f\n\n",config.phase2, config.rayon2, config.alphaInit2);
    fputs("\n\n",Report);
    fputs("Map Visualization : \n",Report);
    fputs("Iris-setosa : A\n",Report);
    fputs("Iris-versicolor : B\n",Report);
    fputs("Iris-viriginica : C\n",Report);

    for (i = 0; i < NNmap.dimension.i; ++i){
        for (j = 0; j < NNmap.dimension.j; ++j){
            fprintf(Report, "%c ", NNmap.N[i][j].label);
        }
        fputs("\n", Report);
    }
    fputs("\n\n",Report);

    fputs("Map Values : \n",Report);
    for (i = 0; i < NNmap.dimension.i; ++i){
        for (j = 0; j < NNmap.dimension.j; ++j){
            for (k = 0; k < config.dimVect; ++k)
                fprintf(Report, "%f ", NNmap.N[i][j].v[k]);
            fputs("; ",Report);
        }
        fputs("\n", Report);
    }
    fputs("\n\n",Report);
    fprintf(Report,"pourcentage :%.2f %%\n", percentage);
    fputs("\n\n",Report);

    fclose(Report);
}

void normalize(iris* irisnum, int dimVect){
    int i;
    irisnum->norme = 0;
    for (i = 0; i < dimVect; ++i)
        irisnum->norme += irisnum->v[i] * irisnum->v[i];
    irisnum->norme = sqrt(irisnum->norme);

    for (i = 0; i < dimVect; ++i)
        irisnum->v[i] /= irisnum->norme;
}

void normalizeAll(iris *DataBase, int DataBaseSize, int dimVect){
    int i;
    for (i = 0; i < DataBaseSize; ++i)
        normalize(&DataBase[i], dimVect);
}

void vectMoyen(iris *Vect_Moyen, iris *DataBase, int DataBaseSize, int dimVect){
    int i, j;
    Vect_Moyen->v = (double*) calloc(dimVect, sizeof(double));
    for (j = 0; j < dimVect; ++j){
        for (i = 0; i<DataBaseSize; ++i)
            Vect_Moyen->v[j] += DataBase[i].v[j];
        Vect_Moyen->v[j] /= DataBaseSize;
    }
    Vect_Moyen->name = "vecteur_moyen";
}

neuron* genererVecteurs(int NbVect, int dimVect, iris* Vect_Moyen, double mini, double maxi){
    int i, j;
    /*double norme;*/
    neuron* pRandomVectors;
    pRandomVectors = (neuron*)malloc(NbVect * sizeof(iris));
    for (i = 0; i < NbVect; ++i){
        pRandomVectors[i].v = (double*)malloc(dimVect*sizeof(double));
        for(j = 0; j < dimVect; ++j)
            pRandomVectors[i].v[j] = (rand()/(double)RAND_MAX ) * ((Vect_Moyen->v[j] + maxi)-(Vect_Moyen->v[j] - mini)) + Vect_Moyen->v[j] - maxi;
    }
    
    return pRandomVectors;
}

void randomizer(int Nblines, int* melange){
    int i, pos, temp;
    for (i = 0; i < Nblines; ++i){
        pos = rand() % (Nblines - i) + i; /* entre 0 et nblines? */
        temp = melange[i];
        melange[i] = melange[pos];
        melange[pos] = temp;
    }
}

double distanceEuclidienne(iris DataBase, double* randomVectors, int dimVect){
    int i;
    double D;
    D = 0;
    for (i = 0; i < dimVect; ++i)
        D += (DataBase.v[i] - randomVectors[i]) * (DataBase.v[i] - randomVectors[i]);
    D = sqrt(D);
    return D;
}

iris chooseRandomVect(iris* DataBase, int Nblines){
    int k;
    k = rand() % Nblines;
    return DataBase[k];
}

coord BMU(Map NNmap, int dimVect, iris vecteurChoisi){
    int i, j;
    coord bmu;
    double D, dist;
    D = distanceEuclidienne(vecteurChoisi, NNmap.N[0][0].v, dimVect);
    bmu.i = 0;
    bmu.j = 0;
    for (i = 0; i < NNmap.dimension.i; ++i){
        for (j = 0; j < NNmap.dimension.j; ++j){
            dist = distanceEuclidienne(vecteurChoisi, NNmap.N[i][j].v, dimVect);
            if (dist < D){
                D =  dist;
                bmu.i = i;
                bmu.j = j;
            }
        }
    }
    return bmu;
}

int verification(Map NNmap, iris* DataBase, int dimVect, int DataBaseSize){
    int k, correct;
    coord bmu;
    bmu.i = 0;
    bmu.j = 0;
    correct = 0;
    for(k = 0; k < DataBaseSize; ++k){
        bmu = BMU(NNmap,dimVect, DataBase[k]);
        if (strcmp(DataBase[k].name, NNmap.N[bmu.i][bmu.j].name) == 0)
            ++correct;
    }
    return correct;
}

coord* Voisinage(coord NNmap_dimension, coord pos, int rayon){
    static coord tailleVois[2];
    tailleVois[0].i = (pos.i - rayon < 0)? 0 : pos.i - rayon;
    tailleVois[1].i = (pos.i + rayon < NNmap_dimension.i)? pos.i + rayon : NNmap_dimension.i - 1;
    tailleVois[0].j = (pos.j - rayon < 0)? 0 : pos.j - rayon;
    tailleVois[1].j = (pos.j + rayon < NNmap_dimension.j)? pos.j + rayon : NNmap_dimension.j - 1;
    return tailleVois;
}

void ModifPoids(Map NNmap, iris vecteurChoisi, coord* TailleVoisinage, double alpha, int dimVect){
    int i,j,k;
    for (i = TailleVoisinage[0].i; i <= TailleVoisinage[1].i; ++i)
        for (j = TailleVoisinage[0].j; j <= TailleVoisinage[1].j; ++j)
            for (k = 0; k < dimVect; ++k)
                    NNmap.N[i][j].v[k] += alpha * (vecteurChoisi.v[k] - NNmap.N[i][j].v[k]);
}

void Weight(int Nbiteration, Map NNmap, int* melange, double alpha, int dimVect, iris* DataBase, int DataBaseSize, int rayon){
    int k, u;
    coord bmu;
    iris vecteurChoisi;
    coord* TailleVoisinage;
    double alpha_init = alpha;
    int rayon_init = rayon;

    for( u = 1; u < Nbiteration; ++u){
        randomizer(DataBaseSize, melange);
        /* 1. Apprentissage */
        for(k = 0; k < DataBaseSize; ++k){
            /*  a. choisir vect */
            vecteurChoisi = DataBase[melange[k]];

            /*  b. Bmu */
            bmu = BMU(NNmap, dimVect, vecteurChoisi);

            /*  c. Voisinage */
            TailleVoisinage = Voisinage(NNmap.dimension, bmu, rayon);
            ModifPoids(NNmap, vecteurChoisi, TailleVoisinage, alpha, dimVect);
        }

        /*  4. alpha */
            alpha = alpha_init * (1.0-((double)u / (double)Nbiteration));

        /*  5. decrementer rayon */
            rayon = rayon_init*(1.0 -((double)u/(double)Nbiteration));
    }
}

/**
 * Lists all files and sub-directories at given path.
 */
int listFiles(char* Filename[]){
    int i = 0;
    struct dirent *dp;

    char cwd[128];
    if(getcwd(cwd, sizeof(cwd)) != NULL){
        DIR *dir = opendir(cwd);

        // Unable to open directory stream
        if (!dir) 
            return 0; 


        while ((dp = readdir(dir)) != NULL && i<30)
        {   
              if (strcmp(dp->d_name + strlen(dp->d_name) - 4, ".txt") == 0 || strstr(dp->d_name,".") == NULL ){
                Filename[i] = dp->d_name;
                ++i;
            }
        }
    }
    return i;
}

int LaunchFromConfig(char* configPath, char* filepath){
    Configurations config;

    int i, j, k;
    int indice;
    int correct;
    
    int DataBaseSize;
    iris* DataBase;
    int* melange;

    Map NNmap;
    iris Vect_Moyen;    
    neuron* RandomVectors;
    
    double min, cmin;

    /* Parse Configuration file */
        readConfig(configPath, &config);
        NNmap.dimension.i = config.dimensionTableau.i;
        NNmap.dimension.j = config.dimensionTableau.j;

    /* Parse CSV File (counting lines) */
        DataBaseSize = countLines(filepath);

        /* Allocate all dynamic memory */
        DataBase = (iris *)malloc(DataBaseSize * sizeof(iris));
        for (i = 0; i < DataBaseSize; ++i)
            DataBase[i].v = (double*)malloc(config.dimVect * sizeof(double));

        melange = (int*)malloc(sizeof(int)*DataBaseSize);
        for (i = 0; i < DataBaseSize; ++i)
            melange[i] = i;

        NNmap.N = (neuron**)malloc(NNmap.dimension.i *sizeof(neuron*));
        for (i = 0; i < NNmap.dimension.i; ++i){
            NNmap.N[i] = (neuron*)malloc(NNmap.dimension.j * sizeof(neuron));
        }
        
        /* Parse CSV File (get values and create vectors) INITIALISATION */
        parseFile(filepath, DataBaseSize, DataBase, config.dimVect);

        /* Normalize vectors and obtain norm */
        normalizeAll(DataBase, DataBaseSize, config.dimVect);

        /* get average Vector then print */
        vectMoyen(&Vect_Moyen, DataBase, DataBaseSize, config.dimVect);

        /* generate n vectors */ 
        srand(time(NULL));
        RandomVectors = genererVecteurs(config.NbVect, config.dimVect, &Vect_Moyen, config.mini, config.maxi);
            
        /* Matrice d'iris ici */
        for (i = 0; i < NNmap.dimension.i; ++i){
            for (j = 0; j < NNmap.dimension.j; ++j){
                NNmap.N[i][j] = RandomVectors[NNmap.dimension.j*i+j];
            }
        }

        /* Adjust weights */
        Weight(config.phase1, NNmap, melange, config.alphaInit1, config.dimVect, DataBase, DataBaseSize, config.rayon1);
        Weight(config.phase2, NNmap, melange, config.alphaInit2, config.dimVect, DataBase, DataBaseSize, config.rayon2);


        /* Finish here */
        for (i = 0; i < NNmap.dimension.i; ++i){
            for (j = 0; j < NNmap.dimension.j; ++j){
                NNmap.N[i][j].label = '.';
            }
        }

    /* etiquettage */
    for (i = 0; i < NNmap.dimension.i; ++i){
        for (j = 0; j<NNmap.dimension.j; ++j){
            min = distanceEuclidienne(DataBase[0], NNmap.N[i][j].v,config.dimVect);
            indice = 1;
            for (k = 1; k < DataBaseSize; k++){
                cmin = distanceEuclidienne(DataBase[k], NNmap.N[i][j].v, config.dimVect);
                if(cmin < min){
                    min = cmin;
                    indice = k;
                }
            }
            NNmap.N[i][j].name = DataBase[indice].name;
        }
    }


    for (i = 0; i < NNmap.dimension.i; ++i){
        for (j = 0;j < NNmap.dimension.j; ++j){
            if (strcmp(NNmap.N[i][j].name, "Iris-setosa")==0)
                NNmap.N[i][j].label = 'A';
            if (strcmp(NNmap.N[i][j].name, "Iris-versicolor")==0)
                NNmap.N[i][j].label = 'B';
            if (strcmp(NNmap.N[i][j].name, "Iris-virginica")==0)
                NNmap.N[i][j].label = 'C';  
        }
    }

    printf("%sIris-setosa : ",KNRM);
    printf("%sA",KRED);
    printf("%s\n", KNRM);
    printf("%sIris-versicolor : ",KNRM);
    printf("%sB",KGRN);
    printf("%s\n", KNRM);
    printf("%sIris-viriginica : ",KNRM);
    printf("%sC",KYEL);
    printf("%s\n\n", KNRM);
    for (i = 0; i < NNmap.dimension.i; ++i){
        for (j = 0; j < NNmap.dimension.j; ++j){
            if (NNmap.N[i][j].label =='A')
                printf("%s", KRED);
            if (NNmap.N[i][j].label =='B')
                printf("%s", KGRN);
            if (NNmap.N[i][j].label =='C')
                printf("%s", KYEL);
            printf(" %c ",NNmap.N[i][j].label);
            printf("%s", KNRM);

        }
        printf("\n");
    }

   /* Done */ 

   /*verification */
    correct = verification(NNmap,DataBase,config.dimVect,DataBaseSize);
    printf("\n%d/%d\n",correct,DataBaseSize);
    double percentage = ((double)correct/(double)DataBaseSize)*100.;
    printf("pourcentage :%.2f %%\n", percentage);

    char* ArgsSave[] = {"Save map","Delete map"};
    switch(menu(2, ArgsSave)){
        int returnValue;
        case 0:
        while (chdir("Reports")!=0){
             makeDir("Reports");
            
        }
        returnValue = system("@cls||clear");
        printf("Report File Name : ");
        char c;
        char reportName[30] = "";
        i = 0;
        do {
            while ((c = getchar())!='\n'){
                if (isalnum(c) || c =='.'){
                reportName[i] = c;
                ++i;
                }
            }
            if ((returnValue = strcmp(reportName, ""))==0){
                fputs ("Must enter valid File Name\n\n", stderr);
                printf("Report File Name : ");
            }
        } while(returnValue==0);

        reportName[i] = '\0';
        writeReport(reportName,NNmap,config, percentage);
        if(chdir("../") == 0){}
        printf("---- Map saved ----\n\n");
        break;

        case 1:
        returnValue = system("@cls||clear");
        printf("---- Map deleted ----\n\n");
        break;
    }
    
    /* Time to Free all allocated memory */
    for (i = 0; i < DataBaseSize; ++i){
        free(DataBase[i].name);
        free(DataBase[i].v);
    }
    free(DataBase);
    
    free(Vect_Moyen.v);

    for(i = 0; i < config.NbVect; ++i)
        free(RandomVectors[i].v);
    free(RandomVectors);

    for (i = 0; i < NNmap.dimension.i; ++i){
        free(NNmap.N[i]);
    }
    free(NNmap.N);

    free(melange);
    return 0;
}


int LaunchFromReport(char* configPath){
    Configurations config;

    int i, j;

    Map NNmap;

    /* Parse Configuration file */
        parseReportConfiguration(configPath, &config);

        NNmap.dimension.i = config.dimensionTableau.i;
        NNmap.dimension.j = config.dimensionTableau.j;
        NNmap.dimVect = config.dimVect;

        NNmap.N = (neuron**)malloc(NNmap.dimension.i *sizeof(neuron*));
        for (i = 0; i < NNmap.dimension.i; ++i){
            NNmap.N[i] = (neuron*)malloc(NNmap.dimension.j * sizeof(neuron));
        }
    
    parseReportMap(configPath, &NNmap);


    printf("%sIris-setosa : ",KNRM);
    printf("%sA",KRED);
    printf("%s\n", KNRM);
    printf("%sIris-versicolor : ",KNRM);
    printf("%sB",KGRN);
    printf("%s\n", KNRM);
    printf("%sIris-viriginica : ",KNRM);
    printf("%sC",KYEL);
    printf("%s\n\n", KNRM);
    for (i = 0; i < NNmap.dimension.i; ++i){
        for (j = 0; j < NNmap.dimension.j; ++j){
            if (NNmap.N[i][j].label =='A')
                printf("%s", KRED);
            if (NNmap.N[i][j].label =='B')
                printf("%s", KGRN);
            if (NNmap.N[i][j].label =='C')
                printf("%s", KYEL);
            printf(" %c ",NNmap.N[i][j].label);
            printf("%s", KNRM);

        }
        printf("\n");
    }

   /* Done */ 

   /*verification */
    double percentage = 0; // in report file
    printf("pourcentage :%.2f %%\n", percentage);
    
    /* Time to Free all allocated memory */
    for (i = 0; i < NNmap.dimension.i; ++i){
        free(NNmap.N[i]);
    }
    free(NNmap.N);

    return 0;
}



int main(void){
    char* Args[] = {"1. Load configuration file and generate map", "2. Load map", "3. Clear reports"};
    int NbFiles;
    int filenum;
    char* configFile[30];
    char* cf;
    char* data;
    char syscommand[30];
    switch (menu ( 3, Args) ){
        case 0 :
        if(system("@cls||clear")!=0){}

        printf("Configuration file name : \n"); //get config file
        NbFiles = listFiles(configFile);
        filenum = menu(NbFiles,configFile);
        cf = configFile[filenum];
        printf("%s\n",configFile[0] );

        printf("\nDatabase file name :\n"); //get iris_data file
        filenum = menu(NbFiles,configFile);
        data = configFile[filenum];
        if(system("@cls||clear")!=0){}
        LaunchFromConfig(cf, data);
        break;
        case 1:

        if( chdir("Reports") == 0){}
        else{
            if(system("@cls||clear")!=0){}
            fputs("---- No Reports ----\n\n", stderr);
            return 0;
        }
        if(system("@cls||clear")!=0){}

        printf("Report files : \n"); //get config file
        NbFiles = listFiles(configFile);
        filenum = menu(NbFiles,configFile);
        cf = configFile[filenum];
        if(system("@cls||clear")!=0){}
        LaunchFromReport(cf);

        break;
        case 2:
        Args[0] = "Delete all reports";
        Args[1] = "Delete single report";
        if( chdir("Reports") == 0){}
        else{
            if(system("@cls||clear")!=0){}
            fputs("---- No Reports ----\n\n", stderr);
            return 0;
        }
        if(system("@cls||clear")!=0){}

        switch(menu(2, Args)){
            case 0:
            if(chdir("../") ==0){}
            if(system("rmdir /Q /S Reports|| rm -r Reports") == 0){
                if(system("@cls||clear")!=0){}
                fputs("---- Reports Deleted ----\n\n", stderr);
            }
            else{
                if(system("@cls||clear")!=0){}
                fputs("---- Cannot Delete Reports ----\n\n", stderr);
            }
            break;
            case 1:
            printf("Report files : \n"); //get config file
            NbFiles = listFiles(configFile);
            if (NbFiles == 1){
                if(chdir("../") == 0){
                    if(system("rmdir /Q /S Reports|| rm -r Reports") == 0){
                    if(system("@cls||clear")!=0){}
                    fputs("---- Report File Deleted ----\n\n", stderr);
                }
                }
            }
            else {
                filenum = menu(NbFiles,configFile);
                cf = configFile[filenum];
                if(system("@cls||clear")!=0){}
                strcpy(syscommand, "del ");
                strcat(syscommand,cf);
                strcat(syscommand,"|| rm -f ");
                strcat(syscommand, cf);
                if(system(syscommand) == 0){
                    if(system("@cls||clear")!=0){}
                    fputs("---- Report File Deleted ----\n\n", stderr);

                }
            }
            break;
        }

    }

}
