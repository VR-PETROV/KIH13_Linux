#include <stdio.h>
#include <tgmath.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////// Цифровой фильтр выполняющий подчеркивание контуров в изображении  ///////////////////

int main( int argc, char * argv[] )

{

unsigned char k1, k2, k3, c1, c2, c3;
long cc1, cc2, cc3;
int f1,f2;

unsigned char * buf1, * buf2;
long dlina_file, poz1, poz2, sm_ob_dan, dl_inf_zg, x_max, y_max, kol_zw_pl, mngt;
long bit_pixel, compres, razr_horiz, razr_vert, razmer, x, y, i, kol_dobaw;
long hx, hy, x_ish, y_ish;



// Задание ширины окна фильтра.
const long okno=3, storona=okno/2;

double h[okno][okno], uroven;


if ((argc!=4) && (argc!=5))  
  {  
    // Командная строка вызова программы записана неправильно.
    // Выдача сообщения об ошибке.
    printf("\n  Oshibka v komandnoi stroke vysova programmy !!! Dolshno byt :      ");
    printf("\n  kih13l   file istochnik   file priemnik  uroven                   ");
    printf("\n                                                                     ");
    printf("\n  gde uroven float znachenie ot 0.1 do 10                            ");
    printf("\n  recomenduemoe znachenie 0.45                                       ");
    printf("\n                                                                     ");
    printf("\n  Esli nado ispolsovat filter-krest togda dobav esho odin argument : ");
    printf("\n  kih13l   file istochnik   file priemnik  uroven 111               ");
    printf("\n                                                                     ");


    // Выход из программы с возвратом кода ошибки.
    return -1;
  }


// Считать третий аргумент командной строки  "Уровень преобразования".
uroven = atof(argv[3]);


if ( argc == 5 )
{
    // Программа имеет 5 аргументов в командной строке.
    // Сформировать  "облегченную" матрицу КИХ-фильтра.
    h[0][0]=0.0;        h[1][0]=-uroven;         h[2][0]=0.0;
    h[0][1]=-uroven;    h[1][1]=4*uroven+1.0;    h[2][1]=-uroven;
    h[0][2]=0.0;        h[1][2]=-uroven;         h[2][2]=0.0;
}

else
{
    // Программа имеет 4 аргумента в командной строке.
    // Сформировать  "обычнуюю" матрицу КИХ-фильтра.
    h[0][0]=-uroven;      h[1][0]=-uroven;         h[2][0]=-uroven;
    h[0][1]=-uroven;      h[1][1]=8*uroven+1.0;    h[2][1]=-uroven;
    h[0][2]=-uroven;      h[1][2]=-uroven;         h[2][2]=-uroven;
}


// Попытка открыть файл для чтения в бинарном режиме.
if ( (f1=open(argv[1], O_RDONLY, 0)) == -1 )
  {
    // Открыть файл не удалось. Выдать сообщние об ошибке.
    fprintf(stderr,"\n  Programma  %s ne moget otkryt file  %s  \n  %s \n  ",argv[0], argv[1], strerror(errno));

    // Выход из программы с возвратом кода ошибки.
    return -1;
  }



// Определить длину в байтах файла хранящего исходное изображение.
dlina_file=lseek(f1,0L,SEEK_END );

// Выделить блок оперативной памяти для хранения исходного изображения.
buf1= (unsigned char *) malloc(dlina_file);

if (buf1==NULL)
{
    // Произошла ошибка при выделении блока оперативной памяти для хранения исходного изображения.
    // Выдать на экран сообщение об ошибке.
    printf("\n Oshibka pri vydelenii bloka pamyati dlya hraneniya ushodnogo file  !!! ");

    // Выход из программы с возвратом кода ошибки.
    return -1;
}

// Позиционировать указатель на начало файла исходного изображения и считать его в память.
lseek(f1,0L,SEEK_SET);
read(f1, buf1, dlina_file);

// Закрыть файл с исходным изображением.
close(f1);

if ('B'!=buf1[0] || 'M'!=buf1[1])
  {
    // Первые два байта исходного файла изображения не равны "BM".
    // Данный файл не является BMP-файлом. Выдать сообщение об ошибке.
    printf("\n OSHIBKA !!!  File  ne BMP !!! ");

    // Освободить ранее выделенный блок оперативной памяти.
    free(buf1);

    // Выход из программы с возвратом кода ошибки.
    return -1;
  }

// Выдать на экран длину BMP-файла в байтах.
printf("\n dlina bmp file = %ld", dlina_file);


// Перейти на позицию 10 в буфере файла исходного изображения и считать четыре байта
// в которых записано смещенрие области данных относительно начала файла.
for (i=0,poz1=10,mngt=1,sm_ob_dan=0; i<4; i++,poz1++,mngt*=256) sm_ob_dan+=mngt*(long) buf1[poz1];


// Выдать на экран смещение области данных BMP-файла.
printf("\n Smechenie oblasti dannyh bmp file  =  %ld", sm_ob_dan );

// Перейти на позицию 14 в буфере файла исходного изображения и считать четыре байта
// в которых записана длина информационного заголовка в байтах.
for (i=0,poz1=14,mngt=1,dl_inf_zg=0; i<4; i++,poz1++,mngt*=256) dl_inf_zg+=mngt*(long) buf1[poz1];

// Выдать длину информационного заголовка.
printf("\n  Dlina inf.zagolovka  bmp file  =  %ld", dl_inf_zg );

// Перейти на позицию 18 в буфере исходного файла изображения и считать четыре байта
// в которых записана длина изображения по горизонтали в пикселях.
for (i=0,poz1=18,mngt=1,x_max=0; i<4; i++,poz1++,mngt*=256)  x_max+=mngt*(long) buf1[poz1];

// Выдать длину изображения по горизонтали в пикселях.
printf("\n  Dlina izobracheniya v pixel = %ld ", x_max);

// Перейти на позицию 22 в буфере исходного изображения и считать четыре байта 
// в которых записана ширина изображения по вертикали в пикселях.
for (i=0,poz1=22,mngt=1,y_max=0; i<4; i++,poz1++,mngt*=256)  y_max+=mngt*(long) buf1[poz1];

// Выдать ширину изображения в пикселях.
printf("\n  Shirina izobracheniya v pixel = %ld ", y_max );

// Перейти на позицию 26 в буфере исходного изображения и считать два байта 
// в которых записано количество цветовых плоскостей.
for (i=0,poz1=26,mngt=1,kol_zw_pl=0; i<2; i++,poz1++,mngt*=256) kol_zw_pl+=mngt*(long) buf1[poz1];

// Выдать количество цветовых плоскостей.
printf("\n  Kolich zwetovyh ploskosteyi = %ld ", kol_zw_pl );

if ( kol_zw_pl != 1 )
  {
    // Количество цветовых плоскостей не равно 1, обработка таких файлов не возможна !!! 
    // Выдать на экран предупреждающее сообщение.
    printf("\n OSHIBKA !!! Kolichestvo zwetovyh ploskostei ne ravno 1, obrabotka ne vozmoshna!!! ");

    // Освободить выделенный блок оперативной памяти.
    free(buf1);

    // Выход из программы с возвратом кода ошибки.
    return -1;
  }


// Перейти на позицию 28 в буфере исходного изображения и считать два байта в которых записано количество бит на пиксель. 
for (i=0,poz1=28,mngt=1,bit_pixel=0; i<2; i++,poz1++,mngt*=256) bit_pixel+=mngt*(long) buf1[poz1];

// Выдать количество бит на пиксель.
printf("\n Kolich bit na pixel = %ld ", bit_pixel);


if ( bit_pixel != 24 )
  {
    // Количество бит на пиксель не равно 24, обработка таких изображений не возможна.
    // Выдать на экран предупреждающее сообщение.
    printf("\n Kolichestvo bit na pixel ne ravno 24 , obrabotka ne vozmoshna !!! ");

    // Освободить блок оперативной памяти выделенный для хранения исходного изображения.
    free(buf1);

    // Выход из программы с возвратом кода ошибки.
    return -1;
  }

// Перейти на позицию 30 в буфере исходного изображения и считать четыре байта в которых записан тип сжатия данных.
for (i=0,poz1=30,mngt=1,compres=0; i<4; i++,poz1++,mngt*=256) compres+=mngt*(long) buf1[poz1];

// Выдать тип сжатия.  
printf("\n  Tip sgatiya dannyh = %ld  ",compres);


if (compres!=0)
  {
    // Данный файл содержит сжатое изображение, обработка таких изображений не возможна.
    // Выдать на экран предупреждающее сообщение.
    printf("\n  Dannyi file sodergit sgatoe izobragenie, obrabotka ne vozvoshna!!! ");

    // Освободить блок оперативной памяти выделенный для хранения исходного изображения.
    free(buf1);
     
    // Выход из программы с возвратом кода ошибки.
    return -1;
  }

// Перейти на позицию  34 в буфере исходного файла изображения и считать четыре байта 
// в которых записан размер графического изображения в байтах.
for (i=0,poz1=34,mngt=1,razmer=0; i<4; i++,poz1++,mngt*=256) razmer+=mngt*(long) buf1[poz1];

// Выдать размер изображения в байтах.
printf(" \n  Razmer izobrageniya v byte = %ld ", razmer);

// Перейти на позицию 38 в буфере исходного изображения и считать четыре байта 
// в которых записано разрешение по горизонтали.
for (i=0,poz1=38,mngt=1,razr_horiz=0; i<4; i++,poz1++,mngt*=256) razr_horiz+=mngt*(long) buf1[poz1];

// Выдать разрешение по горизонтали.
printf(" \n  Razresh po gorizontal = %ld  ", razr_horiz );

// Перейти на позицию 42 в буфере исходного изображения и считать четыре байта в которых записано разрешение по вертикали. 
for(i=0,poz1=42,mngt=1, razr_vert=0; i<4; i++,poz1++,mngt*=256) razr_vert+=mngt*(long) buf1[poz1];

// Выдать разрешение по вертикали. 
printf(" \n  Razresh po vertikal = %ld  ", razr_vert );

// Выделить блок оперативной памяти размером "dlina_file" для записи обработанного изображения.
buf2 = (unsigned char *) malloc(dlina_file);

if (buf2==NULL)
{
    // Ошибка при выделении блока оперативной памяти предназначенного для хранения обработанного изображения.
    printf("\n Oshibkav pri videlenii bloka operativnoi pamyati dlya hraneniya obrabotannogo izobrageniya !!! ");

    // Освободить блок оперативной памяти выделенный для хранения исходного изображения.
    free(buf1);

    // Выход из программы с возвратом кода ошибки.
    return -1;
}


// Считывание заголовка файла исходного изображения и переписывание его в блок 
// динамической памяти выделенный для хранения результирующего изображения.
for (i=0; i<sm_ob_dan; i++) buf2[i]=buf1[i];

// Определить количество добавочных байт в строке изображения (Остаток от деления длины строки на 4). 
kol_dobaw = x_max- x_max/4*4;


// Задать начальное значение позиции записи в буфер результирующего изображения.
poz2=sm_ob_dan;

// В буфер "buf2" записать преобразованное фильтром изображение из буфера "buf1".
for (y=0; y<y_max; y++)
  {
    for (x=0; x<x_max; x++)
    {
        // Обнуление цветовых составляющих элемента изображения с координатами  x,y.
        cc1=0L;
        cc2=0L;
        cc3=0L;

        // Формирование цветовых составляющих элемента изображения с координатими  x,y.
	for (hy=0L; hy<okno; hy++ )
	 for (hx=0L; hx<okno; hx++ )
	    {
	        x_ish=x+hx-storona;
	        y_ish=y+hy-storona;

	        if ( (x_ish>=0) && (x_ish<x_max) && (y_ish>=0) && (y_ish<y_max) )
	        {
		    // В буфере исходного изображения позиционировать указатель на пиксель с координатами  x_ish , y_ish.  
		    poz1=(y_ish*x_max+x_ish)*3L+kol_dobaw*y_ish+sm_ob_dan;

		    // Считать уровень желтого для пикселя  x_ish , y_ish исходного изображения. 
		    k1=buf1[poz1];
		    cc1=cc1+(long)(k1*h[hx][hy]);

		    // Считать уровень пурпурного для пикселя x_ish , y_ish исходного изображения. 
		    poz1++;
		    k2=buf1[poz1];
		    cc2=cc2+(long)(k2*h[hx][hy]);

		    // Считать уровень голубого для пикселя x_ish , y_ish исходного изображения. 
		    poz1++;
		    k3=buf1[poz1];
		    cc3=cc3+(long)(k3*h[hx][hy]);
	        }
	    }

        // Приведение cc1, cc2, cc3 к диапазону 0-255.
        if (cc1<0) cc1=0;
        if (cc1>255) cc1=255;

        if (cc2<0) cc2=0;
        if (cc2>255) cc2=255;

        if (cc3<0) cc3=0;
        if (cc3>255) cc3=255;

        c1=(unsigned char) cc1;
        c2=(unsigned char) cc2;
        c3=(unsigned char) cc3;

	// Запись сформированных цветовых составляющих в буфер результирующего изображения. 
	buf2[poz2]=c1;
	poz2++;

	buf2[poz2]=c2;
	poz2++;

	buf2[poz2]=c3;
	poz2++;
    }

    // Создание добавочных байт в конце строки результирующего изображения.
    for (i=0; i<kol_dobaw; i++) 
    {
	buf2[poz2]=0;
	poz2++;
    }	

  }


// Освободить блок памяти выделенный для хранения исходного изображения.
free(buf1);


// Создать файл для записи результирующего изображения.
if ( (f2=open(argv[2], O_CREAT | O_WRONLY | O_TRUNC | O_SYNC , S_IRWXU | S_IRWXG | S_IRWXO )) == -1 )
  {
    // Не удалось создать файл для записи результирующего изображения.
    // Выдать сообщение об ошибке.
    perror(argv[2]);

    // Освободить выделенный блок оперативной памяти содержащий результирующее изображение.
    free(buf2);

    // Выход из программы с возвратом кода ошибки.
    return -1;
  }

printf("\n File Descriptor f2 = %d",f2);

// Буфер хранящий результирующее изображение записать в выходной файл.
write(f2,buf2,dlina_file);

// Закрыть файл в который записывалось результирующее изображение.
close(f2);

// Освободить блок памяти выделенный для хранения результирующего изображения.
free(buf2);

printf("\n" );


// Завершение работы программы с возвратои 
return 0;

}
