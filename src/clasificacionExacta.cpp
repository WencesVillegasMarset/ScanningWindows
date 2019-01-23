#include "Keypoints.h"
#include "clasificacionExacta.h"
#include "stdlib.h"

bool clasificar(Point2d inicioPatch, Point2d tamanoPatch, char* rutaYemas, string imgName, float criterio){

	std::ifstream myfile(rutaYemas);
	  if (!myfile.is_open()) {
		perror(rutaYemas);  
	    exit(0);
	  }
	vector<Point2d> centroYemas;
	vector<int> radios;
	std::ifstream inputfile(rutaYemas);
	string current_line;
	while(getline(inputfile, current_line)){
		stringstream temp(current_line);
		string single_value;
		string nombreImg;
		Point2d centro;
		int radio;
		for(int i=1;i<=7;i++){
			getline(temp,single_value,',');
			if(i==2) nombreImg = single_value;
			if(i==3) centro.x = atof(single_value.c_str());
			if(i==4) centro.y = atof(single_value.c_str());
			if(i==5) radio = atof(single_value.c_str());
		}
		if(nombreImg==imgName){
			centroYemas.push_back(centro); 
			radios.push_back(radio);
		}
	}
	// sacar la superposicion de los pixeles de yema gt y los pixeles del patch evaluado en este momento
	for(int i=0;i<centroYemas.size();i++){
		float superposicion; // proporcion de overlap
		int matchPixelsX, matchPixelsY, finPatchX, finPatchY, inicioYemaX, inicioYemaY, finYemaX, finYemaY;
		finPatchX = inicioPatch.x + tamanoPatch.x;
		finPatchY = inicioPatch.y + tamanoPatch.y;
		inicioYemaX = centroYemas[i].x - radios[i];
		inicioYemaY = centroYemas[i].y - radios[i];
		finYemaY = centroYemas[i].y + radios[i];
		finYemaX = centroYemas[i].x + radios[i];
		if(inicioPatch.x<inicioYemaX) matchPixelsX = finPatchX-inicioYemaX;
			else matchPixelsX = finYemaX-inicioPatch.x;
		if(inicioPatch.y<inicioYemaY) matchPixelsY = finPatchY-inicioYemaY;
			else matchPixelsY = finYemaY-inicioPatch.y;
		if(matchPixelsY>0 && matchPixelsX>0){
			if(pow(2*radios[i],2)<tamanoPatch.x*tamanoPatch.y){
				superposicion = (matchPixelsY*matchPixelsX)/pow(2*radios[i],2);
			}else{
				superposicion = (matchPixelsY*matchPixelsX)/(tamanoPatch.x*tamanoPatch.y);
			}
		}
		cout<< "la superposición fue de: "<<superposicion<<endl;
		inputfile.close(); //agregado 
		if(superposicion>=criterio)return true; // el patch se etiqueta como 1 para luego comparar la clasificacion
	}

	inputfile.close();//agregado
	return false;


}

