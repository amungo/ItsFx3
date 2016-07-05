function [ X ] = its_convert( file_name, varargin )
% Loads data from 4 channel FX3 receiver into matrix.
%
%   Usage:
% 
%   C = its_convert(filename)
%       Loads all data from file.
%
%   C = its_convert(filename, max_pts)
%       Loads data from file but not more then max_pts.

    file_info = dir(file_name);
    if isempty(file_info)
        error('File open error! Check file name.');
    end
    
    size8 = file_info(1).bytes;
    size8 = size8 - 8;
    size_pts = size8 / 4;
    if ~isempty(varargin)
        user_max_points = varargin{1};
        if user_max_points < size_pts
            size_pts = user_max_points;
        end
    end
    
    fid = fopen( file_name, 'r' );
    if fid < 0
        error('File open error! Check file name.');
    end

    fseek( fid, 8, 'bof' );
    X=zeros(4,size_pts);
    T=fread(fid,'uint16');
    fclose(fid);
    
    ValMx=[1, 3, -1, -3];
    for i=1:size_pts
        u=T(i);
        idx = bitand(u,3,'uint16');
        X(1,i)=ValMx( idx + 1 );
        
        u=bitshift(u,-2,'uint16');
        idx = bitand(u,3,'uint16');
        X(2,i)=ValMx( idx + 1 );
        
        u=bitshift(u,-2,'uint16');
        idx = bitand(u,3,'uint16');
        X(3,i)=ValMx( idx + 1 );
        
        u=bitshift(u,-2,'uint16');
        idx = bitand(u,3,'uint16');
        X(4,i)=ValMx( idx + 1 );
        
    end
    
    
end

