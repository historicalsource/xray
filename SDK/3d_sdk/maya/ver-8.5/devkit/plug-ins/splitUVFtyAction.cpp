//-
// ==========================================================================
// Copyright (C) 1995 - 2006 Autodesk, Inc. and/or its licensors.  All 
// rights reserved.
//
// The coded instructions, statements, computer programs, and/or related 
// material (collectively the "Data") in these files contain unpublished 
// information proprietary to Autodesk, Inc. ("Autodesk") and/or its 
// licensors, which is protected by U.S. and Canadian federal copyright 
// law and by international treaties.
//
// The Data is provided for use exclusively by You. You have the right 
// to use, modify, and incorporate this Data into other products for 
// purposes authorized by the Autodesk software license agreement, 
// without fee.
//
// The copyright notices in the Software and this entire statement, 
// including the above license grant, this restriction and the 
// following disclaimer, must be included in all copies of the 
// Software, in whole or in part, and all derivative works of 
// the Software, unless such copies or derivative works are solely 
// in the form of machine-executable object code generated by a 
// source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND. 
// AUTODESK DOES NOT MAKE AND HEREBY DISCLAIMS ANY EXPRESS OR IMPLIED 
// WARRANTIES INCLUDING, BUT NOT LIMITED TO, THE WARRANTIES OF 
// NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR 
// PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE, OR 
// TRADE PRACTICE. IN NO EVENT WILL AUTODESK AND/OR ITS LICENSORS 
// BE LIABLE FOR ANY LOST REVENUES, DATA, OR PROFITS, OR SPECIAL, 
// DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES, EVEN IF AUTODESK 
// AND/OR ITS LICENSORS HAS BEEN ADVISED OF THE POSSIBILITY 
// OR PROBABILITY OF SUCH DAMAGES.
//
// ==========================================================================
//+

// 
// File: splitUVFtyAction.cpp
//
// Node Factory: splitUVFty
//
// Author: Lonnie Li
//
#include "splitUVFty.h"

// General Includes
//
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>

// Function Sets
//
#include <maya/MFnMesh.h>
#include <maya/MFnSingleIndexedComponent.h>

// Iterators
//
#include <maya/MItMeshPolygon.h>

MStatus splitUVFty::doIt()
//
//	Description:
//		Performs the actual splitUV operation on the given object and UVs
//
{
	MStatus status = MS::kSuccess;

	//////////////////////////////////////
	// Declare our processing variables //
	//////////////////////////////////////

	MString		selUVSet;

	// Face Id and Face Offset map to the selected UVs
	//
	MIntArray	selUVFaceIdMap;
	MIntArray	selUVFaceOffsetMap;

	// Local Vertex Index map to the selected UVs
	//
	MIntArray	selUVLocalVertIdMap;


	///////////////////////////////////////////////////
	// Collect necessary information for the splitUV //
	//												 //
	// - uvSet										 //
	// - faceIds / localVertIds per selected UV		 //
	///////////////////////////////////////////////////

	MFnMesh meshFn( fMesh );
	meshFn.getCurrentUVSetName( selUVSet );

	int i;
	int j;
	int offset = 0;
	int selUVsCount = fSelUVs.length();

	MItMeshPolygon polyIter( fMesh );

	for( i = 0; i < selUVsCount; i++ )
	{
		selUVFaceOffsetMap.append(offset);

		for( polyIter.reset(); !polyIter.isDone(); polyIter.next() )
		{
			if( polyIter.hasUVs() )
			{
				int polyVertCount = polyIter.polygonVertexCount();

				for( j = 0; j < polyVertCount; j++ )
				{
					int UVIndex = 0;
					polyIter.getUVIndex(j, UVIndex);

					if( UVIndex == fSelUVs[i] )
					{
						selUVFaceIdMap.append( polyIter.index() );
						selUVLocalVertIdMap.append(j);
						offset++;
						break;
					}
				}
			}
		}
	}

	// Store total length of the faceId map in the last element of
	// the offset map so that there is a way to get the number of faces
	// sharing each of the selected UVs
	//
	selUVFaceOffsetMap.append(offset);


	/////////////////////////////////
	// Begin the splitUV operation //
	/////////////////////////////////

	int currentUVCount = meshFn.numUVs( selUVSet );

	for( i = 0; i < selUVsCount; i++ )
	{
		// Get the current FaceId map offset
		//
		offset = selUVFaceOffsetMap[i];

		// Get the U and V values of the current UV
		//
		float u;
		float v;
		int uvId = fSelUVs[i];

		meshFn.getUV( uvId, u, v, &selUVSet );

		// Get the number of faces sharing the current UV
		//
		int faceCount = selUVFaceOffsetMap[i + 1] - selUVFaceOffsetMap[i];

		// Arbitrarily choose that the last faceId in the list of faces
		// sharing this UV, will keep the original UV.
		//
		for( j = 0; j < faceCount - 1; j++ )
		{
			meshFn.setUV( currentUVCount, u, v, &selUVSet );

			int localVertId = selUVLocalVertIdMap[offset];
			int faceId = selUVFaceIdMap[offset];

			meshFn.assignUV( faceId, localVertId, currentUVCount, &selUVSet );

			currentUVCount++;
			offset++;
		}
	}

	return status;
}
