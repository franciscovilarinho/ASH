% read B&W image

rootdir = "../data/";
inputfilenameextension  = ".txt";
outputfilenameextension = ".png";

imagefiles = dir( strcat( rootdir, "*", inputfilenameextension ) );
[Nimages N] = size( imagefiles );

if ( Nimages <= 3 )
    RowF = 1; ColF = Nimages;
end
if ( Nimages <= 6 )
    RowF = 3; ColF = ceil(Nimages/2);
end
if ( Nimages <= 9 )
    RowF = 3; ColF = ceil(Nimages/3);
end


figure(1);

for i=1:Nimages
    inputfilename = strcat( rootdir, imagefiles(i).name );
    dotpos = strfind( imagefiles(i).name, '.' );
    rootfilename = strcat( rootdir, imagefiles(i).name(1:dotpos-1) );
    outputfilename = strcat( rootfilename, outputfilenameextension );

    fprintf("[%2d] Reading image file %s\n", i, inputfilename );
    inputimage = uint8( load( inputfilename ) );

    subplot( ColF,  RowF, i );
    imshow( inputimage );
    title( inputfilename );
    
    % Write to bitmap file:
    fprintf("[%2d] Writing image file %s\n", i, outputfilename );
    imwrite(inputimage, outputfilename );
end

return

% Output image:
OutputImageFile = '../data/imageout.png';

% Export to text file, ASCII, one image line per text line
InputTextFile = '../data/datain.txt';
OutputTextFile = '../data/dataout.txt';

i1 = uint8( load( InputTextFile ) );

i2 = uint8( load( OutputTextFile ) );

% Plot original
figure(1);
imshow( i1 );
title('Input image, gray scale');

% Plot final
figure(2);
imshow( i2 );
title('Output image, gray scale');

% Write to bitmap file:
imwrite(i2, OutputImageFile );
        