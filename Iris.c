#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
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
	char etiquette;
};
typedef struct Neuron neurone;

struct NeuralNetworkMap {
	neurone** N;
	coord dimension;
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

neurone* genererVecteurs(int NbVect, int dimVect, iris* Vect_Moyen, double mini, double maxi){
    int i, j;
    /*double norme;*/
    neurone* pRandomVectors;
    pRandomVectors = (neurone*)malloc(NbVect * sizeof(iris));
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

int main(int argc, char *argv[]){
    char* configPath = argv[1];
    char* filepath = argv[2];

    Configurations config;

    int i, j, k;
    int indice;
    int correct;
    
    int DataBaseSize;
    iris* DataBase;
    int* melange;

    Map NNmap;
    iris Vect_Moyen;    
    neurone* RandomVectors;
    
    double min, cmin;
 
    /* error handling */
    if (argc < 3) {
        fprintf (stderr, "Usage %s <Configurations-file.txt> <Filename.txt>\n", argv[0]);
        return 1;
    }

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

    NNmap.N = (neurone**)malloc(NNmap.dimension.i *sizeof(neurone*));
    for (i = 0; i < NNmap.dimension.i; ++i){
        NNmap.N[i] = (neurone*)malloc(NNmap.dimension.j * sizeof(neurone));
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
		    NNmap.N[i][j].etiquette = '.';
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
			    NNmap.N[i][j].etiquette = 'A';
		    if (strcmp(NNmap.N[i][j].name, "Iris-versicolor")==0)
			    NNmap.N[i][j].etiquette = 'B';
		    if (strcmp(NNmap.N[i][j].name, "Iris-virginica")==0)
	            NNmap.N[i][j].etiquette = 'C';	
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
        	if (NNmap.N[i][j].etiquette =='A')
        		printf("%s", KRED);
        	if (NNmap.N[i][j].etiquette =='B')
        		printf("%s", KGRN);
        	if (NNmap.N[i][j].etiquette =='C')
    			printf("%s", KYEL);
            printf(" %c ",NNmap.N[i][j].etiquette);
            printf("%s", KNRM);

        }
        printf("\n");
    }

   /* Done */ 

   /*verification */
    correct = verification(NNmap,DataBase,config.dimVect,DataBaseSize);
    printf("\n%d/%d\n",correct,DataBaseSize);
    printf("pourcentage :%.2f %%\n",((double)correct/(double)DataBaseSize)*100. );
    
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
