// Copyright 2013 Pervasive Displays, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied.  See the License for the specific language
// governing permissions and limitations under the License.


#include <Arduino.h>
#include <limits.h>

//#include <Adafruit_GFX.h>
#include <EPD.h>
#include <S5813A.h>

#include "EPD_GFX.h"


void EPD_GFX::begin() {
	int temperature = this->S5813A.read();

	// erase display
	this->EPD.begin();
	this->EPD.setFactor(temperature);
	this->EPD.clear();
	this->EPD.end();

	// clear buffers to white
	memset(this->old_image, 0, sizeof(this->old_image));
	memset(this->new_image, 0, sizeof(this->new_image));
}


void EPD_GFX::display() {
	int temperature = this->S5813A.read();

	// erase old, display new
	this->EPD.begin();
	this->EPD.setFactor(temperature);
	this->EPD.image_sram(this->old_image, this->new_image);
	this->EPD.end();

	// copy new over to old
	memcpy(this->old_image, this->new_image, sizeof(this->old_image));
}
