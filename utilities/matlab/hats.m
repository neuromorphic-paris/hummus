% hats.m

% Created by Siohoi Ieng
% Institut de la Vision

% Email: siohoi.ieng@inserm.fr

% Last Version: 25/09/2018

% Information: hats calculates the histogram of averaged times surfaces specifically for the n-Cars database scaled to 64x56

function [H] = hats(data,r,tau,dt)
% inputs are 4-column arrays: [x y t p]

% scaled n-Cars database size
W = 64;
H = 56;

% Cell size
cellW = floor(W/10);
cellH = floor(H/10); 

% histogram hc size
hc = 2*r+1;

timeSurface = zeros(hc,hc);
H = zeros(cellH*cellW*hc*hc,1);

for i = 1:size(data,1)
    if (data(i,1) > r && data(i,1) <= cellW*10) && (data(i,2) > r && data(i,2) <= cellH*10)
        
        % reject events that are not in cells: this part needs to be more generic
        if (data(i,1) > 2 && data(i,1) < W-2 && data(i,2) > 3 && data(i,2) < H-3)
            
            % getting the correct cell for a 5-line matrix structure numbered from top to down, left to right
            cellID = cellH*floor((data(i,1)-r-1)/10)+floor((data(i,2)-r-1)/10)+1;
            
            % finding the right event indices
            lst = find(abs(data(1:i-1,1)-data(i,1))<=r & abs(data(1:i-1,2)-data(i,2))<=r & (data(i,3)-data(1:i-1,3))<=dt & data(1:i-1,4)==data(i,4));
            
            if ~isempty(lst)
                for j = 1:size(lst,1)
                    % computing the time surface
                    timeSurface(data(lst(j),2)-data(i,2)+r+1,data(lst(j),1)-data(i,1)+r+1) = timeSurface(data(lst(j),2)-data(i,2)+r+1,data(lst(j),1)-data(i,1)+r+1) + exp(-(data(i,3)-data(lst(j),3))/tau);
                end
                
                % summing time surfaces into histograms and normalising by the number of events
                H((cellID-1)*hc*hc+1:cellID*hc*hc) = H((cellID-1)*hc^2+1:cellID*hc^2) + timeSurface(:) / length(lst);
                
                % resetting time surfaces for next cell
                timeSurface = zeros(hc,hc);
            end
        end  
    end
end