clear all;
% read image & plot original

% Input image:
%InputImageFile = '../data/FEUPin.jpg';

%InputImageFile = '../data/Lena-RGB-512x512.png';
InputImageFile = '../data/bigcat-RGB-512x512.jpg';
%InputImageFile = '../data/smallcat-RGB-512x512.jpg';
%InputImageFile = '../data/spider-RGB-512x512.jpg';

% Seto to zero to use only a 512x512 pixel image
ALLIMAGE = 1;

% Export to text file, ASCII, one image line per text line
OutputTextFile = '../data/datain.txt';

% Export to C include file:
CIncludeFile   = '../aship/datain.h';

i1 = imread( InputImageFile );
figure(1);
imshow( i1 );

imagesize = size( i1 );
NROWS = imagesize(1);
NCOLS = imagesize(2);
% use the whole image:
if ( ALLIMAGE )
  NROWS1 = NROWS;
  NCOLS1 = NCOLS;
  START_ROW = 1;
  START_COL = 1;
else  % Or select a 512x512 image window:
  NROWS1 = 512;
  NCOLS1 = 512;
  START_ROW = 1;
  START_COL = 1;
end


% Convert to grayscale: pixel out = (R + G + B ) / 3
ig_GS = uint8( ( int32(i1(START_ROW:START_ROW+NROWS1-1,START_COL:START_COL+NCOLS1-1,1))+ ...
                 int32(i1(START_ROW:START_ROW+NROWS1-1,START_COL:START_COL+NCOLS1-1,2))+ ...
                 int32(i1(START_ROW:START_ROW+NROWS1-1,START_COL:START_COL+NCOLS1-1,3)) ) / 3 );

% Color channels:
ig_R = uint8( i1(START_ROW:START_ROW+NROWS1-1,START_COL:START_COL+NCOLS1-1,1) );
ig_G = uint8( i1(START_ROW:START_ROW+NROWS1-1,START_COL:START_COL+NCOLS1-1,2) );
ig_B = uint8( i1(START_ROW:START_ROW+NROWS1-1,START_COL:START_COL+NCOLS1-1,3) );

% plot grayscale image:
figure(2);
imshow( ig_GS );
title('Input image - gray scale, 8 bits');

% plot R channel
figure(3);
imshow( ig_R );
title('Input image - RED channel, 8 bits');

% plot G channel
figure(4);
imshow( ig_G );
title('Input image - Green channel, 8 bits');

% plot B channel
figure(5);
imshow( ig_B );
title('Input image - Blue channel, 8 bits');


% Export to text file:
fp = fopen(OutputTextFile, 'w');
for k=1:NROWS1
    fprintf( fp, '%d ', ig_GS(k,:) );
    fprintf( fp, '\n');
end
fclose( fp );


% Export to C include file:
fp = fopen( CIncludeFile, 'w');
fprintf(fp, 'int Min[NROWS][NCOLS] = { \n' );
for k=1:NROWS1 % for each line
    fprintf( fp, '{ \n');
    for l=1:NCOLS1
       if l == NCOLS1
           fprintf( fp, '%3d \n', ig_GS(k,l) );
       else
           fprintf( fp, '%3d, ', ig_GS(k,l)  );
       end
       if mod(l, 32) == 0
           fprintf( fp, '\n' );
       end
    end
    fprintf( fp, '}');
    if k ~= NROWS1
        fprintf( fp, ',\n');
    else
        fprintf( fp, '\n');
    end
end
fprintf(fp, '}; \n');
fclose( fp );

        