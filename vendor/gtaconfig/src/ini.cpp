#include "StdInc.h"
#include <sys/stat.h>

CINI*	LoadINI(CFile *stream)
{
	size_t streamSize = stream->GetSize();

	char *buffer = (char*)malloc( streamSize + 1 );

	stream->Read( buffer, 1, streamSize );
	buffer[ streamSize ] = 0;

	CINI *config = new CINI(buffer);

	free(buffer);

	return config;
}

CINI::CINI(const char *buffer)
{
	CSyntax syntax(buffer, strlen(buffer));
	Entry *section = NULL;

	do
	{
		size_t len;
		const char *token = syntax.ParseToken( &len );

        if ( token )
        {
            // Holy fuck, my past implementation of this was really a danger to human society!

            bool hasProcessedSpecialToken = false;

		    if ( len == 1 )
		    {
			    if ( *token == '[' )
			    {
				    char *name;
				    size_t offset = syntax.GetOffset();

				    // Read section and select it
				    if ( syntax.ScanCharacterEx( ']', true, true, false ) )
                    {
				        size_t tokLen = syntax.GetOffset() - offset - 1;

				        name = (char*)malloc( tokLen + 1 );
				        strncpy( name, token + 1, tokLen );

				        name[tokLen] = 0;

				        section = new Entry( name );

				        entries.push_back( section );

                        hasProcessedSpecialToken = true;
                    }
			    }
                else if ( *token == ';' || *token == '#' )
			    {
				    syntax.ScanCharacter( '\n' );
				    syntax.Seek( -1 );

                    hasProcessedSpecialToken = true;
			    }
		    }
		    
            if ( !hasProcessedSpecialToken )
            {
                if ( section )
		        {
			        char name[256];
			        const char *set;

			        len = std::min(len, (size_t)255);

			        strncpy(name, token, len );
			        name[len] = 0;

			        set = syntax.ParseToken( &len );

			        if ( len == 1 && *set == '=' )
                    {
                        // Read everything until new line.
                        size_t contentSize;
                        const char *lineBuf = syntax.ReadUntilNewLine( &contentSize );

                        std::string strbuf( lineBuf, contentSize );

			            section->Set( name, strbuf.c_str() );
                    }
		        }
            }
        }

	} while ( syntax.GotoNewLine() );
}

CINI::~CINI()
{
	entryList_t::iterator iter;

	for (iter = entries.begin(); iter != entries.end(); iter++)
		delete *iter;
}

CINI::Entry* CINI::GetEntry(const char *entry)
{
	entryList_t::iterator iter;

	for (iter = entries.begin(); iter != entries.end(); iter++)
	{
		if (strcmp((*iter)->m_name, entry) == 0)
			return *iter;
	}

	return NULL;
}

const char* CINI::Get(const char *entry, const char *key)
{
	Entry *section = GetEntry(entry);

	if (!section)
		return NULL;

	return section->Get( key );
}

int CINI::GetInt(const char *entry, const char *key)
{
	const char *value = Get(entry, key);

	if (!value)
		return 0;

	return atoi(value);
}

double CINI::GetFloat(const char *entry, const char *key)
{
	const char *value = Get(entry, key);

	if (!value)
		return 0.0f;

	return atof(value);
}
