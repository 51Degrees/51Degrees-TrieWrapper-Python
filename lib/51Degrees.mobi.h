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


#ifdef __cplusplus
#define EXTERNAL extern "C"
#else
#define EXTERNAL
#endif

// Initialises the memory using the file provided.
EXTERNAL int init(char *fileName, char *properties);

// Returns the offset to a matching device based on the useragent provided.
EXTERNAL int getDeviceOffset(char *userAgent);

// Returns the index of the property requested, or -1 if not available.
EXTERNAL int getPropertyIndex(char *value);

// Takes the results of getDevice and getProperty to return a value.
EXTERNAL char* getValue(int deviceOffset, int propertyIndex);

// Fress the memory.
EXTERNAL void destroy();

// Converts the device offset to a CSV string returning the number of
// characters used.
EXTERNAL int processDeviceCSV(int deviceOffset, char* result, int resultLength);
