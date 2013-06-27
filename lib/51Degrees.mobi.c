/**
 * Copyright (C) 2012 51Degrees.mobi Limited
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "snprintf/snprintf.h"

// Used to map a byte from the data file.
#define BYTE unsigned char

// The maximum value of a byte.
#define BYTE_MAX 255

// The size of the NODE structure. Compilers will byte align
// the result of sizeof to the architecture and therefore
// size of can't be used.
#define NODE_SIZE_BYTES 7

// Type used to represent the start of a node.
typedef struct t_node {
	unsigned int  lookupListPosition;
	unsigned short deviceIndex;
	BYTE numberOfChildren;
} NODE;

// Type used to represent a look up list header.
typedef struct t_lookup_header {
	BYTE lowest;
	BYTE highest;
	BYTE start;
} LOOKUP_HEADER;

// The size of the copyright notice at the top of the data file.
static unsigned int _copyrightSize;

// Pointer to the copyright notice held in the data file.
static char* _copyright;

// The size of the strings data array.
static unsigned int _stringsSize;

// Pointer to the start of the strings data array.
static char* _strings;

// The number of properties contained in the system.
static unsigned int _propertiesCount;

// The size of the profiles data array.
static unsigned int _propertiesSize;

// Pointer to the start of the pointers data array.
static unsigned int* _properties;

// Pointer to the start of the devices data array.
static unsigned int* _devices;

// The size of the deviecs data array.
static unsigned int _devicesSize;

// The size of the memory reserved for lookup lists.
static unsigned int _lookupListSize;

// Pointer to the start of the lookup lists.
static LOOKUP_HEADER* _lookupList;

// Offset in the device data file for the root node.
static NODE* _rootNode;

// The size of the data array containing the nodes.
static unsigned int _treeSize;

// The number of properties to be returned.
static unsigned int _requiredPropertiesCount;

// A list of the required property indexes.
static unsigned int* _requiredProperties;

// A list of pointers to the names of the properties.
static char** _requiredPropertiesNames;

// Reads the strings from the file.
void readStrings(FILE *inputFilePtr) {
	fread(&_stringsSize, sizeof(unsigned int), 1, inputFilePtr);
	_strings = (char*)malloc(_stringsSize);
	fread(_strings, sizeof(BYTE), _stringsSize, inputFilePtr);
}

// Reads the profiles from the file.
void readProperties(FILE *inputFilePtr) {
	fread(&_propertiesSize, sizeof(unsigned int), 1, inputFilePtr);
	_properties = (unsigned int*)malloc(_propertiesSize);
	fread(_properties, sizeof(BYTE), _propertiesSize, inputFilePtr);
	_propertiesCount = _propertiesSize / sizeof(unsigned int);
}

// Reads the profiles from the file.
void readDevices(FILE *inputFilePtr) {
	fread(&_devicesSize, sizeof(unsigned int), 1, inputFilePtr);
	_devices = (unsigned int*)malloc(_devicesSize);
	fread(_devices, sizeof(BYTE), _devicesSize, inputFilePtr);
}

// Reads the lookups from the input file provided.
void readLookupList(FILE *inputFilePtr) {
	fread(&_lookupListSize, sizeof(unsigned int), 1, inputFilePtr);
	_lookupList = (LOOKUP_HEADER*)malloc(_lookupListSize);
	fread(_lookupList, sizeof(BYTE), _lookupListSize, inputFilePtr);
}

// Reads the nodes byte array into memory.
void readNodes(FILE *inputFilePtr) {
	fread(&_treeSize, sizeof(unsigned int), 1, inputFilePtr);
	_rootNode = (NODE*)malloc(_treeSize);
	if (_rootNode > 0)
		fread(_rootNode, sizeof(BYTE), _treeSize, inputFilePtr);
}

// Reads the copyright message into memory.
void readCopyright(FILE *inputFilePtr) {
	fread(&_copyrightSize, sizeof(unsigned int), 1, inputFilePtr);
	_copyright = (char*)malloc(_copyrightSize);
	fread(_copyright, sizeof(BYTE), _copyrightSize, inputFilePtr);
}

// Fress the memory.
void destroy(void) {
	if (_requiredProperties > 0) free(_requiredProperties);
	if (_rootNode > 0) free(_rootNode);
	if (_lookupList > 0) free(_lookupList);
	if (_devices > 0) free(_devices);
	if (_properties > 0) free(_properties);
	if (_strings > 0) free(_strings);
}

// Reads the version value from the start of the file and returns
// 0 if the file is in a format that can be read by this code.
int readVersion(FILE *inputFilePtr) {
	unsigned short version;
	fread(&version, sizeof(unsigned short), 1, inputFilePtr);
	return (version != 1);
}

// Reads the input file into memory returning 1 if it
// was read unsuccessfully, otherwise 0.
int readFile(char* fileName) {
	FILE *inputFilePtr;
	int failed = 0;

	// Open the file and hold on to the pointer.
	inputFilePtr = fopen(fileName, "rb");

	// If the file didn't open return -1.
	if (inputFilePtr == NULL)
		return -1;

	// Read the various data segments if the version is
	// one we can read.
	failed = readVersion(inputFilePtr);
	if (failed == 0)
	{
		readCopyright(inputFilePtr);
		readStrings(inputFilePtr);
		readProperties(inputFilePtr);
		readDevices(inputFilePtr);
		readLookupList(inputFilePtr);
		readNodes(inputFilePtr);
		failed = failed | (_rootNode == 0);
	}
	fclose(inputFilePtr);

	return failed;
}

// Returns the index of the property requested, or -1 if not available.
int getPropertyIndexRange(char *start, char *end) {
	unsigned int i = 0;
	for(i = 0; i < _propertiesCount; i++) {
		if(strncmp(
			_strings + *(_properties + i),
			start,
			end - start) == 0) {
			return i;
		}
	}
	return -1;
}

// Initialises the properties provided.
void initSpecificProperties(char* properties) {
	char *start, *end;
	int propertyIndex, currentIndex = 0;

	// Count the number of valid properties.
	_requiredPropertiesCount = 0;
	start = properties;
	end = properties - 1;
	do {
		end++;
		if (*end == '|' || *end == ',' || *end == '\0') {
			// Check the property we've just reached is valid and
			// if it is then increase the count.
			if (getPropertyIndexRange(start, end) > 0)
				_requiredPropertiesCount++;
			start = end + 1;
		}
	} while (*end != '\0');

	// Create enough memory for the properties.
	_requiredProperties = (unsigned int*)malloc(_requiredPropertiesCount * sizeof(int));
	_requiredPropertiesNames = (char**)malloc(_requiredPropertiesCount * sizeof(char**));

	// Initialise the requiredProperties array.
	start = properties;
	end = properties - 1;
	do {
		end++;
		if (*end == '|' || *end == ',' || *end == '\0') {
			// If this is a valid property add it to the list.
			propertyIndex = getPropertyIndexRange(start, end);
			if (propertyIndex > 0) {
				*(_requiredProperties + currentIndex) = propertyIndex;
				*(_requiredPropertiesNames + currentIndex) = _strings + *(_properties + propertyIndex);
				currentIndex++;
			}
			start = end + 1;
		}

	} while (*end != '\0');
}

// Initialises all the available properties.
void initAllProperties() {
	unsigned int i;

	// Set to include all properties.
	_requiredPropertiesCount = _propertiesCount;

	// Create enough memory for the properties.
	_requiredProperties = (unsigned int*)malloc(_requiredPropertiesCount * sizeof(int));
	_requiredPropertiesNames = (char**)malloc(_requiredPropertiesCount * sizeof(char**));

	// Add all the available properties.
	for(i = 0; i < _propertiesCount; i++) {
		*(_requiredProperties + i) = i;
		*(_requiredPropertiesNames + i) = _strings + *(_properties + i);
	}
}

// Initialises the memory using the file provided.
int init(char* fileName, char* properties) {
	// Read the data from the file provided.
	if (readFile(fileName) != 0)
		return -1;

	// If no properties are provided then use all of them.
	if (properties == NULL || strlen(properties) == 0)
		initAllProperties();
	else
		initSpecificProperties(properties);

	return 0;
}

// Returns the index of the property requested, or -1 if not available.
int getPropertyIndex(char *value) {
	unsigned int i = 0;
	for(i = 0; i < _propertiesCount; i++) {
		if(strcmp(
			_strings + *(_properties + i),
			value) == 0) {
			return i;
		}
	}
	return -1;
}

// Returns the index of the child of the current node based on
// the value of the current character being compared.
BYTE __inline  getChildIndex(char value, unsigned int lookupListPosition) {
	LOOKUP_HEADER *lookup= (LOOKUP_HEADER*)(((BYTE*)_lookupList) + lookupListPosition);
	if (value < lookup->lowest ||
		value > lookup->highest)
		return BYTE_MAX;

	// Return the child index.
	return *(&lookup->start + value - lookup->lowest);
}

// Returns a pointer to the next node.
NODE* getNextNode(NODE* currentNode, BYTE childIndex) {
	unsigned int *position = (unsigned int*)(((BYTE*)currentNode) + NODE_SIZE_BYTES + (childIndex * sizeof(unsigned int)));
	return (NODE*)(((BYTE*)_rootNode) + *position);
}

// Returns the offset in the devices array to the matched device.
unsigned short getChildDevice(char* userAgent, NODE* node) {
	BYTE childIndex;

	// Get the child index for the current node.
	childIndex = getChildIndex(*userAgent, node->lookupListPosition);

	// Check the child index isn't too large.
	if (childIndex >= node->numberOfChildren)
		return node->deviceIndex;

	// Move to the next child.
	return getChildDevice(
		&userAgent[1],
		getNextNode(node, childIndex));
}

// Returns the offset to a matching device based on the useragent provided.
int getDeviceOffset(char* userAgent) {
	return getChildDevice(userAgent, _rootNode) * _propertiesCount;
}

// Takes the results of getDevice and getProperty to return a value.
char* getValue(int deviceOffset, int propertyIndex) {
	return _strings + *(_devices + deviceOffset + propertyIndex);
}

// Process device properties into a CSV string.
int processDeviceCSV(int deviceOffset, char* result, int resultLength) {
	char* currentPos = result;
	char* endPos = result + resultLength;
	unsigned int i;

	// If no properties return nothing.
	if (_requiredPropertiesCount == 0)
		return sprintf(currentPos, "");

	// Process each line of data using the relevant value separator. In this case, a pipe.
	for(i = 0; i < _requiredPropertiesCount; i++) {
		// Add the next property to the buffer.
		currentPos += snprintf(
			currentPos,
			(int)(endPos - currentPos),
			"%s|%s\n",
			*(_requiredPropertiesNames + i),
			getValue(deviceOffset, *(_requiredProperties + i)));

		// Check to see if buffer is filled in which case return -1.
		if (currentPos >= endPos)
			return -1;
	}

	// Return the length of the string buffer used.
	return (int)(currentPos - result);
}
