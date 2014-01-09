#include "effect.h"

const GLfloat TimeStep = 0.2f;
const GLfloat shiftRange = 1.0f;
const GLfloat TensionCoefficient = 1.6f;
int stepCount = 0;
//M3DVector3f TotalDisplacement = {0.0f, 0.0f, 0.0f};

void initEnvironment() {
	outFileClothModel();
	inFileClothModel();
	M3DVector3f shift;
	m3dLoadVector3(shift, -WIDNUMBER*EDGELENGTH/2, 200.0f, - GAPLENGTH/2);
	shiftClothModelPosition(shift);
	initManModel();
	initClothModel();
	getClothModelCenter();
}

void getForce(int currentPtc) {
	getInternalForce(currentPtc);
	//getCollisionForce(currentPtc);
	M3DVector3f gravity = {0.0f, -ClothLnk[currentPtc].Mass, 0.0f};
	m3dScaleVector3(gravity, 0.2f);
	addForce(currentPtc, gravity);

	m3dScaleVector3(ClothLnk[currentPtc].force, DampCoefficient);

	//if (currentPtc==55)
	//	printf("[%d].force		{%.1f, %.1f, %.1f} \n", currentPtc, ClothLnk[currentPtc].force[0], ClothLnk[currentPtc].force[1], ClothLnk[currentPtc].force[2]);
}

//verlet x = vt + 1/2*at^2
void nextPosition(int currentPtc) {
	M3DVector3f temp;
	//get last shift: + vt
	m3dSubtractVectors3(temp, ClothLnk[currentPtc].position, ClothLnk[currentPtc].pre_position);
	m3dAddVectors3(ClothLnk[currentPtc].next_position, ClothLnk[currentPtc].position, temp);
	//if (currentPtc==55)
	//	printf("	[%d].shift		{%.1f, %.1f, %.1f} \n", currentPtc, shift[0], shift[1], shift[2]);

	//get accumulation: + 1/2*at^2
	m3dCopyVector3(temp, ClothLnk[currentPtc].force);
	m3dScaleVector3(temp, (GLfloat)TimeStep*TimeStep*AccumulateStiff/ClothLnk[currentPtc].Mass/2);
	m3dAddVectors3(ClothLnk[currentPtc].next_position, ClothLnk[currentPtc].next_position, temp);

	//collide
	//if (adjustPositionByCollision(ClothLnk[currentPtc].next_position) ) {
	//	m3dCopyVector3(ClothLnk[currentPtc].next_position, ClothLnk[currentPtc].position);
	//}

	adjustPositionByCollision(ClothLnk[currentPtc].position, ClothLnk[currentPtc].next_position);

	M3DVector3f shift;
	m3dSubtractVectors3(shift, ClothLnk[currentPtc].next_position, ClothLnk[currentPtc].position);
	GLfloat length = (GLfloat)m3dGetVectorLength(shift);
	if (length >= shiftRange ) {
		m3dScaleVector3(shift, 1.0f/length);
		m3dAddVectors3(ClothLnk[currentPtc].next_position, ClothLnk[currentPtc].position, shift);
	}

}

void needStop(int currentPtc) {
	if ((GLfloat)m3dGetVectorLength(ClothLnk[currentPtc].force) <= minForce) {
		m3dLoadVector3(ClothLnk[currentPtc].force, 0.0f, 0.0f, 0.0f);
		m3dCopyVector3(ClothLnk[currentPtc].pre_position, ClothLnk[currentPtc].position);
	}
}

void refreshPosition() {
	for (int i = 0; i < PTCAMT; i ++) {
		m3dCopyVector3(ClothLnk[i].pre_position, ClothLnk[i].position);
		m3dCopyVector3(ClothLnk[i].position, ClothLnk[i].next_position);
	}
}

void sewClothModel() {
	M3DVector3f temp, displacement;
	for (int i = 0; i < PTCAMT; i ++) {
		m3dLoadVector3(displacement, 0.0f, 0.0f, 0.0f);
		clearForce(i);
		getInternalForce(i);
		//sew together
		getTensionForce(i, ClothCenter, 0.01f);
		m3dScaleVector3(ClothLnk[i].force, TensionCoefficient);
		nextPosition(i);
		m3dSubtractVectors3(temp, ClothLnk[i].next_position, ClothLnk[i].position);
		m3dAddVectors3(displacement, displacement, temp);
	}
	m3dScaleVector3(displacement, (GLfloat)800000/PTCAMT);
	//printf(" total displace: %.1f %.1f %.1f\n", displacement[0], displacement[1], displacement[2]);
	m3dAddVectors3(ClothCenter, ClothCenter, displacement);
	//m3dLoadVector3(TotalDisplacement, 0.0f, 0.0f, 0.0f);
	refreshPosition();

}

void simulation(void) {
	//if (true) {
	//	sewClothModel();
	//} else if (stepCount == 200){
	//	for (int i = 0; i < PTCAMT; i ++) {

	//		clearForce(i);
	//		m3dCopyVector3(ClothLnk[i].next_position, ClothLnk[i].position);
	//	}
	//} else {
		for (int i = 0; i < PTCAMT; i ++) {
			clearForce(i);
			getForce(i);
			//if ((i>=44 && i<=46)||(i>=54 && i<=56)||(i>=64 && i<=66))
			//	printf("[%d].force	 	{%.1f, %.1f, %.1f} \n", i, ClothLnk[i].force[0], ClothLnk[i].force[1], ClothLnk[i].force[2]);
			//printf("	ClothLnk[%d].force = {%f, %f, %f} \n", i, ClothLnk[i].force[0], ClothLnk[i].force[1], ClothLnk[i].force[2]);
		}
	//}
	for (int i = 0; i < PTCAMT; i ++) {
		nextPosition(i);
	}
	refreshPosition();
	stepCount ++;
}
