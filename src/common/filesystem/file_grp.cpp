/*
** file_grp.cpp
**
**---------------------------------------------------------------------------
** Copyright 1998-2009 Randy Heit
** Copyright 2005-2009 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
**
*/

#include "resourcefile.h"
#include "printf.h"

//==========================================================================
//
//
//
//==========================================================================

struct GrpHeader
{
	uint32_t		Magic[3];
	uint32_t		NumLumps;
};

struct GrpLump
{
	union
	{
		struct
		{
			char		Name[12];
			uint32_t		Size;
		};
		char NameWithZero[13];
	};
};


//==========================================================================
//
// Build GRP file
//
//==========================================================================

class FGrpFile : public FUncompressedFile
{
public:
	FGrpFile(const char * filename, FileReader &file);
	bool Open(bool quiet, LumpFilterInfo* filter);
};


//==========================================================================
//
// Initializes a Build GRP file
//
//==========================================================================

FGrpFile::FGrpFile(const char *filename, FileReader &file)
: FUncompressedFile(filename, file)
{
}

//==========================================================================
//
// Open it
//
//==========================================================================

bool FGrpFile::Open(bool quiet, LumpFilterInfo*)
{
	GrpHeader header;

	Reader.Read(&header, sizeof(header));
	NumLumps = LittleLong(header.NumLumps);
	
	GrpLump *fileinfo = new GrpLump[NumLumps];
	Reader.Read (fileinfo, NumLumps * sizeof(GrpLump));

	Lumps.Resize(NumLumps);

	int Position = sizeof(GrpHeader) + NumLumps * sizeof(GrpLump);

	for(uint32_t i = 0; i < NumLumps; i++)
	{
		Lumps[i].Owner = this;
		Lumps[i].Position = Position;
		Lumps[i].LumpSize = LittleLong(fileinfo[i].Size);
		Position += fileinfo[i].Size;
		Lumps[i].Flags = 0;
		fileinfo[i].NameWithZero[12] = '\0';	// Be sure filename is null-terminated
		Lumps[i].LumpNameSetup(fileinfo[i].NameWithZero);
	}
	GenerateHash();
	delete[] fileinfo;
	return true;
}


//==========================================================================
//
// File open
//
//==========================================================================

FResourceFile *CheckGRP(const char *filename, FileReader &file, bool quiet, LumpFilterInfo* filter)
{
	char head[12];

	if (file.GetLength() >= 12)
	{
		file.Seek(0, FileReader::SeekSet);
		file.Read(&head, 12);
		file.Seek(0, FileReader::SeekSet);
		if (!memcmp(head, "KenSilverman", 12))
		{
			auto rf = new FGrpFile(filename, file);
			if (rf->Open(quiet, filter)) return rf;

			file = std::move(rf->Reader); // to avoid destruction of reader
			delete rf;
		}
	}
	return NULL;
}

