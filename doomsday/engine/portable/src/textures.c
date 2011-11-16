/**\file textures.c
 *\section License
 * License: GPL
 * Online License Link: http://www.gnu.org/licenses/gpl.html
 *
 *\author Copyright © 2010-2011 Daniel Swanson <danij@dengine.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "de_base.h"
#include "de_console.h"

#include "m_misc.h" // For M_NumDigits
#include "fs_util.h" // For F_PrettyPath
#include "r_world.h" // For ddMapSetup

#include "gl_texmanager.h"
#include "pathdirectory.h"

#include "textures.h"

/**
 * TextureRecord. Stores metadata for a unique Texture in the collection.
 */
typedef struct {
    /// Namespace-unique identifier chosen by the owner of the collection.
    int uniqueId;

    /// Path to the data resource which contains/wraps the loadable texture data.
    Uri* resourcePath;

    /// The defined texture instance (if any).
    texture_t* texture;
} texturerecord_t;

typedef struct {
    /// PathDirectory containing mappings between names and unique texture records.
    PathDirectory* directory;

    /// LUT which translates namespace-unique-ids to their associated textureid_t (if any).
    /// Index with uniqueId - uniqueIdBase.
    int uniqueIdBase;
    boolean uniqueIdMapDirty;
    uint uniqueIdMapSize;
    textureid_t* uniqueIdMap;
} texturenamespace_t;

D_CMD(ListTextures);
D_CMD(InspectTexture);
#if _DEBUG
D_CMD(PrintTextureStats);
#endif

static int iterateDirectory(texturenamespaceid_t namespaceId,
    int (*callback)(PathDirectoryNode* node, void* paramaters), void* paramaters);

static Uri* emptyUri;

// LUT which translates textureid_t to PathDirectoryNode*. Index with textureid_t-1
static uint textureIdMapSize;
static PathDirectoryNode** textureIdMap;

// Texture namespace set.
static texturenamespace_t namespaces[TEXTURENAMESPACE_COUNT];

void Textures_Register(void)
{
    C_CMD("inspecttexture", NULL, InspectTexture)
    C_CMD("listtextures", NULL, ListTextures)
#if _DEBUG
    C_CMD("texturestats", NULL, PrintTextureStats)
#endif
}

static __inline PathDirectory* getDirectoryForNamespaceId(texturenamespaceid_t id)
{
    assert(VALID_TEXTURENAMESPACEID(id));
    return namespaces[id-TEXTURENAMESPACE_FIRST].directory;
}

static texturenamespaceid_t namespaceIdForDirectory(PathDirectory* pd)
{
    texturenamespaceid_t id;
    assert(pd);

    for(id = TEXTURENAMESPACE_FIRST; id <= TEXTURENAMESPACE_LAST; ++id)
    {
        if(namespaces[id-TEXTURENAMESPACE_FIRST].directory == pd) return id;
    }
    // Only reachable if attempting to find the id for a Texture that is not
    // in the collection, or the collection has not yet been initialized.
    Con_Error("Textures::namespaceIdForDirectory: Failed to determine id for directory %p.", (void*)pd);
    exit(1); // Unreachable.
}

static __inline boolean validTextureId(textureid_t id)
{
    return (id != NOTEXTUREID && id <= textureIdMapSize);
}

static PathDirectoryNode* getDirectoryNodeForBindId(textureid_t id)
{
    if(!validTextureId(id)) return NULL;
    return textureIdMap[id-1/*1-based index*/];
}

static textureid_t findBindIdForDirectoryNode(const PathDirectoryNode* node)
{
    uint i;
    /// \optimize (Low priority) do not use a linear search.
    for(i = 0; i < textureIdMapSize; ++i)
    {
        if(textureIdMap[i] == node)
            return (textureid_t)(i+1); // 1-based index.
    }
    return NOTEXTUREID; // Not linked.
}

static __inline texturenamespaceid_t namespaceIdForDirectoryNode(const PathDirectoryNode* node)
{
    return namespaceIdForDirectory(PathDirectoryNode_Directory(node));
}

/// @return  Newly composed path for @a node. Must be released with Str_Delete()
static __inline ddstring_t* composePathForDirectoryNode(const PathDirectoryNode* node, char delimiter)
{
    return PathDirectory_ComposePath(PathDirectoryNode_Directory(node), node, Str_New(), NULL, delimiter);
}

/// @return  Newly composed Uri for @a node. Must be released with Uri_Delete()
static Uri* composeUriForDirectoryNode(const PathDirectoryNode* node)
{
    const ddstring_t* namespaceName = Textures_NamespaceName(namespaceIdForDirectoryNode(node));
    ddstring_t* path = composePathForDirectoryNode(node, TEXTURES_PATH_DELIMITER);
    Uri* uri = Uri_NewWithPath2(Str_Text(path), RC_NULL);
    Uri_SetScheme(uri, Str_Text(namespaceName));
    Str_Delete(path);
    return uri;
}

/// \assume textureIdMap has been initialized and is large enough!
static void unlinkDirectoryNodeFromBindIdMap(const PathDirectoryNode* node)
{
    textureid_t id = findBindIdForDirectoryNode(node);
    if(!validTextureId(id)) return; // Not linked.
    textureIdMap[id - 1/*1-based index*/] = NULL;
}

/// \assume uniqueIdMap has been initialized and is large enough!
static int linkRecordInUniqueIdMap(PathDirectoryNode* node, void* paramaters)
{
    const texturerecord_t* record = (texturerecord_t*)PathDirectoryNode_UserData(node);
    const texturenamespaceid_t namespaceId = namespaceIdForDirectory(PathDirectoryNode_Directory(node));
    texturenamespace_t* tn = &namespaces[namespaceId-TEXTURENAMESPACE_FIRST];
    tn->uniqueIdMap[record->uniqueId - tn->uniqueIdBase] = findBindIdForDirectoryNode(node);
    return 0; // Continue iteration.
}

/// \assume uniqueIdMap is large enough if initialized!
static int unlinkRecordInUniqueIdMap(PathDirectoryNode* node, void* paramaters)
{
    const texturerecord_t* record = (texturerecord_t*)PathDirectoryNode_UserData(node);
    const texturenamespaceid_t namespaceId = namespaceIdForDirectory(PathDirectoryNode_Directory(node));
    texturenamespace_t* tn = &namespaces[namespaceId-TEXTURENAMESPACE_FIRST];
    if(tn->uniqueIdMap)
    {
        tn->uniqueIdMap[record->uniqueId - tn->uniqueIdBase] = NOTEXTUREID;
    }
    return 0; // Continue iteration.
}

/**
 * @defgroup validateTextureUriFlags  Validate Texture Uri Flags
 * @{
 */
#define VTUF_ALLOW_NAMESPACE_ANY 0x1 /// The namespace of the uri may be of zero-length; signifying "any namespace".
#define VTUF_NO_URN             0x2 /// Do not accept a URN.
/**@}*/

/**
 * @param uri  Uri to be validated.
 * @param flags  @see validateTextureUriFlags
 * @param quiet  @c true= Do not output validation remarks to the log.
 * @return  @c true if @a Uri passes validation.
 */
static boolean validateTextureUri2(const Uri* uri, int flags, boolean quiet)
{
    texturenamespaceid_t namespaceId;
    const ddstring_t* namespaceString;

    if(!uri || Str_IsEmpty(Uri_Path(uri)))
    {
        if(!quiet)
        {
            ddstring_t* uriStr = Uri_ToString(uri);
            Con_Message("Invalid path '%s' in Texture uri \"%s\".\n", Str_Text(Uri_Path(uri)), Str_Text(uriStr));
            Str_Delete(uriStr);
        }
        return false;
    }

    // If this is a URN we extract the namespace from the path.
    if(!Str_CompareIgnoreCase(Uri_Scheme(uri), "urn"))
    {
        if(flags & VTUF_NO_URN) return false;
        namespaceString = Uri_Path(uri);
    }
    else
    {
        namespaceString = Uri_Scheme(uri);
    }

    namespaceId = Textures_ParseNamespace(Str_Text(namespaceString));
    if(!((flags & VTUF_ALLOW_NAMESPACE_ANY) && namespaceId == TN_ANY) &&
       !VALID_TEXTURENAMESPACEID(namespaceId))
    {
        if(!quiet)
        {
            ddstring_t* uriStr = Uri_ToString(uri);
            Con_Message("Unknown namespace in Texture uri \"%s\".\n", Str_Text(uriStr));
            Str_Delete(uriStr);
        }
        return false;
    }

    return true;
}

static boolean validateTextureUri(const Uri* uri, int flags)
{
    return validateTextureUri2(uri, flags, false);
}

/**
 * Given a directory and path, search the Textures collection for a match.
 *
 * @param directory  Namespace-specific PathDirectory to search in.
 * @param path  Path of the texture to search for.
 * @return  Found DirectoryNode else @c NULL
 */
static PathDirectoryNode* findDirectoryNodeForPath(PathDirectory* texDirectory, const char* path)
{
    return PathDirectory_Find(texDirectory, PCF_NO_BRANCH|PCF_MATCH_FULL, path, TEXTURES_PATH_DELIMITER);
}

/// \assume @a uri has already been validated and is well-formed.
static PathDirectoryNode* findDirectoryNodeForUri(const Uri* uri)
{
    texturenamespaceid_t namespaceId;
    PathDirectoryNode* node = NULL;
    const char* path;

    if(!Str_CompareIgnoreCase(Uri_Scheme(uri), "urn"))
    {
        // This is a URN of the form; urn:namespacename:uniqueid
        char* uid;
        namespaceId = Textures_ParseNamespace(Str_Text(Uri_Path(uri)));
        uid = strchr(Str_Text(Uri_Path(uri)), ':');
        if(uid)
        {
            textureid_t texId = Textures_TextureForUniqueId(namespaceId,
                strtol(uid +1/*skip namespace delimiter*/, 0, 0));
            if(texId != NOTEXTUREID)
            {
                return getDirectoryNodeForBindId(texId);
            }
        }
        return NULL;
    }

    // This is a URI.
    namespaceId = Textures_ParseNamespace(Str_Text(Uri_Scheme(uri)));
    path = Str_Text(Uri_Path(uri));
    if(namespaceId != TN_ANY)
    {
        // Caller wants a texture in a specific namespace.
        node = findDirectoryNodeForPath(getDirectoryForNamespaceId(namespaceId), path);
    }
    else
    {
        // Caller does not care which namespace.
        // Check for the texture in these namespaces in priority order.
        static const texturenamespaceid_t order[] = {
            TN_SPRITES,
            TN_TEXTURES,
            TN_FLATS,
            TN_PATCHES,
            TN_SYSTEM,
            TN_DETAILS,
            TN_REFLECTIONS,
            TN_MASKS,
            TN_MODELSKINS,
            TN_MODELREFLECTIONSKINS,
            TN_LIGHTMAPS,
            TN_FLAREMAPS,
        };
        int n = 0;
        do
        {
            node = findDirectoryNodeForPath(getDirectoryForNamespaceId(order[n]), path);
        } while(!node && order[++n] != MN_ANY);
    }
    return node;
}

static void destroyTexture(texture_t* tex)
{
    assert(tex);

    GL_ReleaseGLTexturesByTexture(tex);
    switch(Textures_Namespace(Textures_Id(tex)))
    {
    case TN_SYSTEM:
    case TN_DETAILS:
    case TN_REFLECTIONS:
    case TN_MASKS:
    case TN_MODELSKINS:
    case TN_MODELREFLECTIONSKINS:
    case TN_LIGHTMAPS:
    case TN_FLAREMAPS:
    case TN_FLATS: break;

    case TN_TEXTURES: {
        patchcompositetex_t* pcTex = (patchcompositetex_t*)Texture_DetachUserData(tex);
        if(pcTex)
        {
            Str_Free(&pcTex->name);
            if(pcTex->patches) free(pcTex->patches);
            free(pcTex);
        }
        break;
    }
    case TN_SPRITES: {
        spritetex_t* sprTex = (spritetex_t*)Texture_DetachUserData(tex);
        if(sprTex) free(sprTex);
        break;
    }
    case TN_PATCHES: {
        patchtex_t* pTex = (patchtex_t*)Texture_DetachUserData(tex);
        if(pTex) free(pTex);
        break;
    }
    default:
        Con_Error("Textures::destroyTexture: Internal error, invalid namespace id %i.", (int)Textures_Namespace(Textures_Id(tex)));
        exit(1); // Unreachable.
    }
    Texture_Delete(tex);
}

static int destroyBoundTexture(PathDirectoryNode* node, void* paramaters)
{
    texturerecord_t* record = (texturerecord_t*)PathDirectoryNode_UserData(node);
    if(record && record->texture)
    {
        destroyTexture(record->texture), record->texture = NULL;
    }
    return 0; // Continue iteration.
}

static int destroyRecord(PathDirectoryNode* node, void* paramaters)
{
    texturerecord_t* record = (texturerecord_t*)PathDirectoryNode_UserData(node);
    if(record)
    {
        if(record->texture)
        {
#if _DEBUG
            Uri* uri = composeUriForDirectoryNode(node);
            ddstring_t* path = Uri_ToString(uri);
            Con_Message("Warning:Textures::destroyRecord: Record for \"%s\" still has Texture data!\n", Str_Text(path));
            Str_Delete(path);
            Uri_Delete(uri);
#endif
            destroyTexture(record->texture);
        }

        if(record->resourcePath)
        {
            Uri_Delete(record->resourcePath);
        }

        unlinkDirectoryNodeFromBindIdMap(node);
        unlinkRecordInUniqueIdMap(node, NULL/*no paramaters*/);

        PathDirectoryNode_DetachUserData(node);
        free(record);
    }
    return 0; // Continue iteration.
}

static int destroyTextureAndRecord(PathDirectoryNode* node, void* paramaters)
{
    destroyBoundTexture(node, paramaters);
    destroyRecord(node, paramaters);
    return 0; // Continue iteration.
}

void Textures_Init(void)
{
    int i;

    VERBOSE( Con_Message("Initializing Textures collection...\n") )

    emptyUri = Uri_New();

    textureIdMap = NULL;
    textureIdMapSize = 0;

    for(i = 0; i < TEXTURENAMESPACE_COUNT; ++i)
    {
        texturenamespace_t* tn = &namespaces[i];
        tn->directory = PathDirectory_New();
        tn->uniqueIdBase = 0;
        tn->uniqueIdMapSize = 0;
        tn->uniqueIdMap = NULL;
        tn->uniqueIdMapDirty = false;
    }
}

void Textures_Shutdown(void)
{
    int i;

    Textures_Clear();

    for(i = 0; i < TEXTURENAMESPACE_COUNT; ++i)
    {
        texturenamespace_t* tn = &namespaces[i];

        PathDirectory_Iterate(tn->directory, PCF_NO_BRANCH, NULL, PATHDIRECTORY_NOHASH, destroyRecord);
        PathDirectory_Delete(tn->directory);
        namespaces[i].directory = NULL;

        if(!tn->uniqueIdMap) continue;
        free(tn->uniqueIdMap), tn->uniqueIdMap = NULL;
        tn->uniqueIdBase = 0;
        tn->uniqueIdMapSize = 0;
        tn->uniqueIdMapDirty = false;
    }

    // Clear the bindId to PathDirectoryNode LUT.
    if(textureIdMap)
    {
        free(textureIdMap), textureIdMap = NULL;
    }
    textureIdMapSize = 0;

    Uri_Delete(emptyUri), emptyUri = NULL;
}

texturenamespaceid_t Textures_ParseNamespace(const char* str)
{
    static const struct namespace_s {
        const char* name;
        size_t nameLen;
        texturenamespaceid_t id;
    } namespaces[TEXTURENAMESPACE_COUNT+1] = {
        // Ordered according to a best guess of occurance frequency.
        { TN_TEXTURES_NAME, sizeof(TN_TEXTURES_NAME)-1, TN_TEXTURES },
        { TN_FLATS_NAME, sizeof(TN_FLATS_NAME)-1, TN_FLATS },
        { TN_SPRITES_NAME, sizeof(TN_SPRITES_NAME)-1, TN_SPRITES },
        { TN_PATCHES_NAME, sizeof(TN_PATCHES_NAME)-1, TN_PATCHES },
        { TN_SYSTEM_NAME, sizeof(TN_SYSTEM_NAME)-1, TN_SYSTEM },
        { TN_DETAILS_NAME, sizeof(TN_DETAILS_NAME)-1, TN_DETAILS },
        { TN_REFLECTIONS_NAME, sizeof(TN_REFLECTIONS_NAME)-1, TN_REFLECTIONS },
        { TN_MASKS_NAME, sizeof(TN_MASKS_NAME)-1, TN_MASKS },
        { TN_MODELSKINS_NAME, sizeof(TN_MODELSKINS_NAME)-1, TN_MASKS },
        { TN_MODELREFLECTIONSKINS_NAME, sizeof(TN_MODELREFLECTIONSKINS_NAME)-1, TN_MODELREFLECTIONSKINS },
        { TN_LIGHTMAPS_NAME, sizeof(TN_LIGHTMAPS_NAME)-1, TN_LIGHTMAPS },
        { TN_FLAREMAPS_NAME, sizeof(TN_FLAREMAPS_NAME)-1, TN_FLAREMAPS },
        { NULL }
    };
    const char* end;
    size_t len, n;

    // Special case: zero-length string means "any namespace".
    if(!str || 0 == (len = strlen(str))) return TN_ANY;

    // Stop comparing characters at the first occurance of ':'
    end = strchr(str, ':');
    if(end) len = end - str;

    for(n = 0; namespaces[n].name; ++n)
    {
        if(len < namespaces[n].nameLen) continue;
        if(strnicmp(str, namespaces[n].name, len)) continue;
        return namespaces[n].id;
    }

    return TN_INVALID; // Unknown.
}

const ddstring_t* Textures_NamespaceName(texturenamespaceid_t id)
{
    static const ddstring_t namespaceNames[TEXTURENAMESPACE_COUNT+1] = {
        /* No namespace name */         { "" },
        /* TN_SYSTEM */                 { TN_SYSTEM_NAME },
        /* TN_FLATS */                  { TN_FLATS_NAME  },
        /* TN_TEXTURES */               { TN_TEXTURES_NAME },
        /* TN_SPRITES */                { TN_SPRITES_NAME },
        /* TN_PATCHES */                { TN_PATCHES_NAME },
        /* TN_DETAILS */                { TN_DETAILS_NAME },
        /* TN_REFLECTIONS */            { TN_REFLECTIONS_NAME },
        /* TN_MASKS */                  { TN_MASKS_NAME },
        /* TN_MODELSKINS */             { TN_MODELSKINS_NAME },
        /* TN_MODELREFLECTIONSKINS */   { TN_MODELREFLECTIONSKINS_NAME },
        /* TN_LIGHTMAPS */              { TN_LIGHTMAPS_NAME },
        /* TN_FLAREMAPS */              { TN_FLAREMAPS_NAME }
    };
    if(VALID_TEXTURENAMESPACEID(id))
        return namespaceNames + 1 + (id - TEXTURENAMESPACE_FIRST);
    return namespaceNames + 0;
}

uint Textures_Size(void)
{
    return textureIdMapSize;
}

uint Textures_Count(texturenamespaceid_t namespaceId)
{
    PathDirectory* texDirectory;
    if(!VALID_TEXTURENAMESPACEID(namespaceId) || !Textures_Size()) return 0;
    texDirectory = getDirectoryForNamespaceId(namespaceId);
    if(!texDirectory) return 0;
    return PathDirectory_Size(texDirectory);
}

void Textures_Clear(void)
{
    if(!Textures_Size()) return;

    Textures_ClearNamespace(TN_ANY);
    GL_PruneTextureVariantSpecifications();
}

void Textures_ClearRuntime(void)
{
    if(!Textures_Size()) return;

    Textures_ClearNamespace(TN_FLATS);
    Textures_ClearNamespace(TN_TEXTURES);
    Textures_ClearNamespace(TN_PATCHES);
    Textures_ClearNamespace(TN_SPRITES);
    Textures_ClearNamespace(TN_DETAILS);
    Textures_ClearNamespace(TN_REFLECTIONS);
    Textures_ClearNamespace(TN_MASKS);
    Textures_ClearNamespace(TN_MODELSKINS);
    Textures_ClearNamespace(TN_MODELREFLECTIONSKINS);
    Textures_ClearNamespace(TN_LIGHTMAPS);
    Textures_ClearNamespace(TN_FLAREMAPS);

    GL_PruneTextureVariantSpecifications();
}

void Textures_ClearSystem(void)
{
    if(!Textures_Size()) return;

    Textures_ClearNamespace(TN_SYSTEM);
    GL_PruneTextureVariantSpecifications();
}

void Textures_ClearNamespace(texturenamespaceid_t texNamespace)
{
    texturenamespaceid_t from, to, iter;
    if(!Textures_Size()) return;

    if(texNamespace == TN_ANY)
    {
        from = TEXTURENAMESPACE_FIRST;
        to   = TEXTURENAMESPACE_LAST;
    }
    else if(VALID_TEXTURENAMESPACEID(texNamespace))
    {
        from = to = texNamespace;
    }
    else
    {
        Con_Error("Textures_ClearNamespace: Invalid texture namespace %i.", (int) texNamespace);
        exit(1); // Unreachable.
    }

    for(iter = from; iter <= to; ++iter)
    {
        texturenamespace_t* tn = &namespaces[iter - TEXTURENAMESPACE_FIRST];
        PathDirectory_Iterate(tn->directory, PCF_NO_BRANCH, NULL, PATHDIRECTORY_NOHASH, destroyTextureAndRecord);
        PathDirectory_Clear(tn->directory);
        tn->uniqueIdMapDirty = true;
    }
}

void Textures_Release(texture_t* tex)
{
    /// Stub.
    GL_ReleaseGLTexturesByTexture(tex);
    /// \fixme Update any Materials (and thus Surfaces) which reference this.
}

texture_t* Textures_ToTexture(textureid_t id)
{
    PathDirectoryNode* node;
    if(!validTextureId(id))
    {
#if _DEBUG
        if(id != NOTEXTUREID)
        {
            Con_Message("Warning:Textures::ToTexture: Failed to locate texture for id #%i, returning NULL.\n", id);
        }
#endif
        return NULL;
    }
    node = getDirectoryNodeForBindId(id);
    if(!node) return NULL;
    return ((texturerecord_t*)PathDirectoryNode_UserData(node))->texture;
}

typedef struct {
    int minId, maxId;
} finduniqueidbounds_params_t;

static int findUniqueIdBounds(PathDirectoryNode* node, void* paramaters)
{
    const texturerecord_t* record = (texturerecord_t*)PathDirectoryNode_UserData(node);
    finduniqueidbounds_params_t* p = (finduniqueidbounds_params_t*)paramaters;
    if(record->uniqueId < p->minId) p->minId = record->uniqueId;
    if(record->uniqueId > p->maxId) p->maxId = record->uniqueId;
    return 0; // Continue iteration.
}

static void rebuildUniqueIdMap(texturenamespaceid_t namespaceId)
{
    texturenamespace_t* tn = &namespaces[namespaceId - TEXTURENAMESPACE_FIRST];
    texturenamespaceid_t localNamespaceId = namespaceId;
    finduniqueidbounds_params_t p;
    assert(VALID_TEXTURENAMESPACEID(namespaceId));

    if(!tn->uniqueIdMapDirty) return;

    // Determine the size of the LUT.
    p.minId = DDMAXINT;
    p.maxId = DDMININT;
    iterateDirectory(namespaceId, findUniqueIdBounds, (void*)&p);

    if(p.minId > p.maxId)
    {
        // None found.
        tn->uniqueIdBase = 0;
        tn->uniqueIdMapSize = 0;
    }
    else
    {
        tn->uniqueIdBase = p.minId;
        tn->uniqueIdMapSize = p.maxId - p.minId + 1;
    }

    // Construct and (re)populate the LUT.
    tn->uniqueIdMap = (textureid_t*)realloc(tn->uniqueIdMap, sizeof *tn->uniqueIdMap * tn->uniqueIdMapSize);
    if(!tn->uniqueIdMap && tn->uniqueIdMapSize)
        Con_Error("Textures::rebuildUniqueIdMap: Failed on (re)allocation of %lu bytes resizing the map.", (unsigned long) sizeof *tn->uniqueIdMap * tn->uniqueIdMapSize);

    if(tn->uniqueIdMapSize)
    {
        memset(tn->uniqueIdMap, NOTEXTUREID, sizeof *tn->uniqueIdMap * tn->uniqueIdMapSize);
        iterateDirectory(namespaceId, linkRecordInUniqueIdMap, (void*)&localNamespaceId);
    }
    tn->uniqueIdMapDirty = false;
}

textureid_t Textures_TextureForUniqueId(texturenamespaceid_t namespaceId, int uniqueId)
{
    if(VALID_TEXTURENAMESPACEID(namespaceId))
    {
        texturenamespace_t* tn = &namespaces[namespaceId - TEXTURENAMESPACE_FIRST];

        rebuildUniqueIdMap(namespaceId);
        if(tn->uniqueIdMap && uniqueId >= tn->uniqueIdBase &&
           (unsigned)(uniqueId - tn->uniqueIdBase) <= tn->uniqueIdMapSize)
        {
            return tn->uniqueIdMap[uniqueId - tn->uniqueIdBase];
        }
    }
    return NOTEXTUREID; // Not found.
}

textureid_t Textures_ResolveUri2(const Uri* uri, boolean quiet)
{
    PathDirectoryNode* node;
    if(!uri || !Textures_Size()) return NOTEXTUREID;

    if(!validateTextureUri2(uri, VTUF_ALLOW_NAMESPACE_ANY, true /*quiet please*/))
    {
#if _DEBUG
        ddstring_t* uriStr = Uri_ToString(uri);
        Con_Message("Warning:Textures::ResolveUri: Uri \"%s\" failed to validate, returing NULL.\n", Str_Text(uriStr));
        Str_Delete(uriStr);
#endif
        return NOTEXTUREID;
    }

    // Perform the search.
    node = findDirectoryNodeForUri(uri);
    if(node)
    {
        // If we have bound a texture - it can provide the id.
        texturerecord_t* record = (texturerecord_t*)PathDirectoryNode_UserData(node);
        assert(record);
        if(record->texture)
        {
            textureid_t id = Texture_PrimaryBind(record->texture);
            if(validTextureId(id)) return id;
        }
        // Oh well, look it up then.
        return findBindIdForDirectoryNode(node);
    }

    // Not found.
    if(!quiet && !ddMapSetup) // Do not announce during map setup.
    {
        ddstring_t* path = Uri_ToString(uri);
        Con_Message("Textures::ResolveUri: \"%s\" not found!\n", Str_Text(path));
        Str_Delete(path);
    }
    return NOTEXTUREID;
}

textureid_t Textures_ResolveUri(const Uri* uri)
{
    return Textures_ResolveUri2(uri, !(verbose >= 1)/*log warnings if verbose*/);
}

textureid_t Textures_ResolveUriCString2(const char* path, boolean quiet)
{
    if(path && path[0])
    {
        Uri* uri = Uri_NewWithPath2(path, RC_NULL);
        textureid_t id = Textures_ResolveUri2(uri, quiet);
        Uri_Delete(uri);
        return id;
    }
    return NOTEXTUREID;
}

textureid_t Textures_ResolveUriCString(const char* path)
{
    return Textures_ResolveUriCString2(path, !(verbose >= 1)/*log warnings if verbose*/);
}

textureid_t Textures_Declare(const Uri* uri, int uniqueId, const Uri* resourcePath)
{
    PathDirectoryNode* node;
    texturerecord_t* record;
    textureid_t id;
    boolean releaseTexture = false;
    assert(uri);

    // We require a properly formed uri (but not a urn - this is a path).
    if(!validateTextureUri2(uri, VTUF_NO_URN, (verbose >= 1)))
    {
        ddstring_t* uriStr = Uri_ToString(uri);
        Con_Message("Warning, failed to create Texture \"%s\", ignoring.\n", Str_Text(uriStr));
        Str_Delete(uriStr);
        return NOTEXTUREID;
    }

    // Have we already created a binding for this?
    node = findDirectoryNodeForUri(uri);
    if(node)
    {
        record = (texturerecord_t*)PathDirectoryNode_UserData(node);
        assert(record);
        id = findBindIdForDirectoryNode(node);
    }
    else
    {
        // A new binding.
        texturenamespaceid_t namespaceId = Textures_ParseNamespace(Str_Text(Uri_Scheme(uri)));
        texturenamespace_t* tn = &namespaces[namespaceId - TEXTURENAMESPACE_FIRST];
        ddstring_t path;
        int i, pathLen;

        // Ensure the path is lowercase.
        pathLen = Str_Length(Uri_Path(uri));
        Str_Init(&path);
        Str_Reserve(&path, pathLen);
        for(i = 0; i < pathLen; ++i)
        {
            Str_AppendChar(&path, tolower(Str_At(Uri_Path(uri), i)));
        }

        record = (texturerecord_t*)malloc(sizeof *record);
        if(!record)
            Con_Error("Textures::CreateWithDimensions: Failed on allocation of %lu bytes for new TextureRecord.", (unsigned long) sizeof *record);
        record->texture = NULL;
        record->resourcePath = NULL;
        record->uniqueId = uniqueId;

        node = PathDirectory_Insert(tn->directory, Str_Text(&path), TEXTURES_PATH_DELIMITER);
        PathDirectoryNode_AttachUserData(node, record);

        // We'll need to rebuild the unique id map too.
        tn->uniqueIdMapDirty = true;

        id = textureIdMapSize + 1; // 1-based identfier
        // Link it into the id map.
        textureIdMap = (PathDirectoryNode**) realloc(textureIdMap, sizeof *textureIdMap * ++textureIdMapSize);
        if(!textureIdMap)
            Con_Error("Textures::CreateWithDimensions: Failed on (re)allocation of %lu bytes enlarging bindId to PathDirectoryNode LUT.", (unsigned long) sizeof *textureIdMap * textureIdMapSize);
        textureIdMap[id - 1] = node;

        Str_Free(&path);
    }

    /**
     * (Re)configure this binding.
     */

    // We don't care whether these identfiers are truely unique. Our only
    // responsibility is to release textures when they change.
    if(record->uniqueId != uniqueId)
    {
        texturenamespaceid_t namespaceId = namespaceIdForDirectory(PathDirectoryNode_Directory(node));
        texturenamespace_t* tn = &namespaces[namespaceId - TEXTURENAMESPACE_FIRST];

        record->uniqueId = uniqueId;
        releaseTexture = true;

        // We'll need to rebuild the id map too.
        tn->uniqueIdMapDirty = true;
    }

    if(resourcePath)
    {
        if(!record->resourcePath)
        {
            record->resourcePath = Uri_NewCopy(resourcePath);
            releaseTexture = true;
        }
        else if(!Uri_Equality(record->resourcePath, resourcePath))
        {
            Uri_Copy(record->resourcePath, resourcePath);
            releaseTexture = true;
        }
    }
    else if(record->resourcePath)
    {
        Uri_Delete(record->resourcePath);
        record->resourcePath = NULL;
        releaseTexture = true;
    }

    if(releaseTexture && record->texture)
    {
        // The mapped resource is being replaced, so release any existing Texture.
        /// \todo Only release if this Texture is bound to only this binding.
        Textures_Release(record->texture);
    }

    return id;
}

texture_t* Textures_CreateWithDimensions(textureid_t id, int flags, int width, int height, void* userData)
{
    PathDirectoryNode* node = getDirectoryNodeForBindId(id);
    texturerecord_t* record;
    if(!node)
    {
        Con_Message("Warning, failed to create Texture #%u (invalid id), ignoring.\n", id);
        return NULL;
    }

    record = (texturerecord_t*)PathDirectoryNode_UserData(node);
    assert(record);
    if(record->texture)
    {
        /// \todo Do not update textures here (not enough knowledge). We should instead
        /// return an invalid reference/signal and force the caller to implement the
        /// necessary update logic.
        texture_t* tex = record->texture;
#if _DEBUG
        Uri* uri = Textures_ComposeUri(id);
        ddstring_t* path = Uri_ToString(uri);
        Con_Message("Warning:Textures::CreateWithDimensions: A Texture with uri \"%s\" already exists, returning existing.\n", Str_Text(path));
        Str_Delete(path);
        Uri_Delete(uri);
#endif
        Texture_SetFlags(tex, flags);
        Texture_SetDimensions(tex, width, height);
        Texture_AttachUserData(tex, userData);
        /// \fixme Materials and Surfaces should be notified of this!
        return tex;
    }

    // A new texture.
    return record->texture = Texture_NewWithDimensions(flags, id, width, height, userData);
}

texture_t* Textures_Create(textureid_t id, int flags, void* userData)
{
    return Textures_CreateWithDimensions(id, flags, 0, 0, userData);
}

int Textures_UniqueId(textureid_t id)
{
    PathDirectoryNode* node = getDirectoryNodeForBindId(id);
    if(!node)
        Con_Error("Textures::UniqueId: Passed invalid textureId #%u.", id);
    return ((texturerecord_t*)PathDirectoryNode_UserData(node))->uniqueId;
}

const Uri* Textures_ResourcePath(textureid_t id)
{
    PathDirectoryNode* node = getDirectoryNodeForBindId(id);
    if(!node) return emptyUri;
    return ((texturerecord_t*)PathDirectoryNode_UserData(node))->resourcePath;
}

textureid_t Textures_Id(texture_t* tex)
{
    if(!tex)
    {
#if _DEBUG
        Con_Message("Warning:Textures::Id: Attempted with invalid reference [%p], returning invalid id.\n", (void*)tex);
#endif
        return NOTEXTUREID;
    }
    return Texture_PrimaryBind(tex);
}

texturenamespaceid_t Textures_Namespace(textureid_t id)
{
    PathDirectoryNode* node = getDirectoryNodeForBindId(id);
    if(!node)
    {
#if _DEBUG
        if(id != NOTEXTUREID)
        {
            Con_Message("Warning:Textures::Namespace: Attempted with unbound textureId #%u, returning null-object.\n", id);
        }
#endif
        return TN_ANY;
    }
    return namespaceIdForDirectoryNode(node);
}

ddstring_t* Textures_ComposePath(textureid_t id)
{
    PathDirectoryNode* node = getDirectoryNodeForBindId(id);
    if(!node)
    {
#if _DEBUG
        Con_Message("Warning:Textures::ComposePath: Attempted with unbound textureId #%u, returning null-object.\n", id);
#endif
        return Str_New();
    }
    return composePathForDirectoryNode(node, TEXTURES_PATH_DELIMITER);
}

Uri* Textures_ComposeUri(textureid_t id)
{
    PathDirectoryNode* node = getDirectoryNodeForBindId(id);
    if(!node)
    {
#if _DEBUG
        if(id != NOTEXTUREID)
        {
            Con_Message("Warning:Textures::ComposeUri: Attempted with unbound textureId #%u, returning null-object.\n", id);
        }
#endif
        return Uri_New();
    }
    return composeUriForDirectoryNode(node);
}

Uri* Textures_ComposeUrn(textureid_t id)
{
    PathDirectoryNode* node = getDirectoryNodeForBindId(id);
    const ddstring_t* namespaceName;
    const texturerecord_t* record;
    Uri* uri = Uri_New();
    ddstring_t path;
    if(!node)
    {
#if _DEBUG
        if(id != NOTEXTUREID)
        {
            Con_Message("Warning:Textures::ComposeUrn: Attempted with unbound textureId #%u, returning null-object.\n", id);
        }
#endif
        return uri;
    }
    record = (texturerecord_t*)PathDirectoryNode_UserData(node);
    assert(record);
    namespaceName = Textures_NamespaceName(namespaceIdForDirectoryNode(node));
    Str_Init(&path);
    Str_Reserve(&path, Str_Length(namespaceName) +1/*delimiter*/ +M_NumDigits(DDMAXINT));
    Str_Appendf(&path, "%s:%i", Str_Text(namespaceName), record->uniqueId);
    Uri_SetScheme(uri, "urn");
    Uri_SetPath(uri, Str_Text(&path));
    Str_Free(&path);
    return uri;
}

typedef struct {
    int (*definedCallback)(texture_t* tex, void* paramaters);
    int (*declaredCallback)(textureid_t id, void* paramaters);
    void* paramaters;
} iteratedirectoryworker_params_t;

static int iterateDirectoryWorker(PathDirectoryNode* node, void* paramaters)
{
    iteratedirectoryworker_params_t* p = (iteratedirectoryworker_params_t*)paramaters;
    texturerecord_t* record = (texturerecord_t*)PathDirectoryNode_UserData(node);
    assert(node && p && record);
    if(p->definedCallback)
    {
        if(record->texture)
        {
            return p->definedCallback(record->texture, p->paramaters);
        }
    }
    else
    {
        textureid_t texId = NOTEXTUREID;

        // If we have bound a texture it can provide the id.
        if(record->texture)
            texId = Texture_PrimaryBind(record->texture);
        // Otherwise look it up.
        if(!validTextureId(texId))
            texId = findBindIdForDirectoryNode(node);
        // Sanity check.
        assert(validTextureId(texId));

        return p->declaredCallback(texId, p->paramaters);
    }
    return 0; // Continue iteration.
}

static int iterateDirectory(texturenamespaceid_t namespaceId,
    int (*callback)(PathDirectoryNode* node, void* paramaters), void* paramaters)
{
    texturenamespaceid_t from, to, iter;
    int result = 0;

    if(VALID_TEXTURENAMESPACEID(namespaceId))
    {
        from = to = namespaceId;
    }
    else
    {
        from = TEXTURENAMESPACE_FIRST;
        to   = TEXTURENAMESPACE_LAST;
    }

    for(iter = from; iter <= to; ++iter)
    {
        PathDirectory* texDirectory = getDirectoryForNamespaceId(iter);
        result = PathDirectory_Iterate2(texDirectory, PCF_NO_BRANCH, NULL, PATHDIRECTORY_NOHASH, callback, paramaters);
        if(result) break;
    }
    return result;
}

int Textures_Iterate2(texturenamespaceid_t namespaceId,
    int (*callback)(texture_t* tex, void* paramaters), void* paramaters)
{
    iteratedirectoryworker_params_t p;
    if(!callback) return 0;
    p.definedCallback = callback;
    p.declaredCallback = NULL;
    p.paramaters = paramaters;
    return iterateDirectory(namespaceId, iterateDirectoryWorker, &p);
}

int Textures_Iterate(texturenamespaceid_t namespaceId,
    int (*callback)(texture_t* tex, void* paramaters))
{
    return Textures_Iterate2(namespaceId, callback, NULL);
}

int Textures_IterateDeclared2(texturenamespaceid_t namespaceId,
    int (*callback)(textureid_t textureId, void* paramaters), void* paramaters)
{
    iteratedirectoryworker_params_t p;
    if(!callback) return 0;
    p.declaredCallback = callback;
    p.definedCallback = NULL;
    p.paramaters = paramaters;
    return iterateDirectory(namespaceId, iterateDirectoryWorker, &p);
}

int Textures_IterateDeclared(texturenamespaceid_t namespaceId,
    int (*callback)(textureid_t textureId, void* paramaters))
{
    return Textures_IterateDeclared2(namespaceId, callback, NULL);
}

static void printTextureInfo(texture_t* tex)
{
    Uri* uri = Textures_ComposeUri(Textures_Id(tex));
    ddstring_t* path = Uri_ToString(uri);
    //int variantIdx = 0;

    Con_Printf("Texture \"%s\" [%p] uid:%u origin:%s\nDimensions: %d x %d\n",
        F_PrettyPath(Str_Text(path)), (void*) tex, (uint) Textures_Id(tex),
        Texture_IsCustom(tex)? "addon" : "game",
        Texture_Width(tex), Texture_Height(tex));

    //Texture_IterateVariants(tex, printVariantInfo, (void*)&variantIdx);

    Str_Delete(path);
    Uri_Delete(uri);
}

static void printTextureOverview(PathDirectoryNode* node, boolean printNamespace)
{
    texturerecord_t* record = (texturerecord_t*) PathDirectoryNode_UserData(node);
    textureid_t texId = findBindIdForDirectoryNode(node);
    int numUidDigits = MAX_OF(3/*uid*/, M_NumDigits(Textures_Size()));
    Uri* uri = record->texture? Textures_ComposeUri(texId) : Uri_New();
    const ddstring_t* path = (printNamespace? Uri_ToString(uri) : Uri_Path(uri));

    Con_FPrintf(!record->texture? CPF_LIGHT : CPF_WHITE,
        "%-*s %*u %s\n", printNamespace? 22 : 14, F_PrettyPath(Str_Text(path)),
        numUidDigits, texId, !record->texture? "unknown" : Texture_IsCustom(record->texture)? "addon" : "game");

    Uri_Delete(uri);
    if(printNamespace)
        Str_Delete((ddstring_t*)path);
}

/**
 * \todo A horridly inefficent algorithm. This should be implemented in PathDirectory
 * itself and not force users of this class to implement this sort of thing themselves.
 * However this is only presently used for the texture search/listing console commands
 * so is not hugely important right now.
 */
typedef struct {
    char delimiter;
    const char* like;
    uint idx;
    PathDirectoryNode** storage;
} collectdirectorynodeworker_paramaters_t;

static int collectDirectoryNodeWorker(PathDirectoryNode* node, void* paramaters)
{
    collectdirectorynodeworker_paramaters_t* p = (collectdirectorynodeworker_paramaters_t*)paramaters;

    if(p->like && p->like[0])
    {
        ddstring_t* path = composePathForDirectoryNode(node, p->delimiter);
        int delta = strnicmp(Str_Text(path), p->like, strlen(p->like));
        Str_Delete(path);
        if(delta) return 0; // Continue iteration.
    }

    if(p->storage)
    {
        p->storage[p->idx++] = node;
    }
    else
    {
        ++p->idx;
    }

    return 0; // Continue iteration.
}

static PathDirectoryNode** collectDirectoryNodes(texturenamespaceid_t namespaceId,
    const char* like, uint* count, PathDirectoryNode** storage)
{
    collectdirectorynodeworker_paramaters_t p;
    texturenamespaceid_t fromId, toId, iterId;

    if(VALID_TEXTURENAMESPACEID(namespaceId))
    {
        // Only consider textures in this namespace.
        fromId = toId = namespaceId;
    }
    else
    {
        // Consider textures in any namespace.
        fromId = TEXTURENAMESPACE_FIRST;
        toId   = TEXTURENAMESPACE_LAST;
    }

    p.delimiter = TEXTURES_PATH_DELIMITER;
    p.like = like;
    p.idx = 0;
    p.storage = storage;
    for(iterId  = fromId; iterId <= toId; ++iterId)
    {
        PathDirectory* texDirectory = getDirectoryForNamespaceId(iterId);
        PathDirectory_Iterate2(texDirectory, PCF_NO_BRANCH|PCF_MATCH_FULL, NULL,
            PATHDIRECTORY_NOHASH, collectDirectoryNodeWorker, (void*)&p);
    }

    if(storage)
    {
        storage[p.idx] = NULL; // Terminate.
        if(count) *count = p.idx;
        return storage;
    }

    if(p.idx == 0)
    {
        if(count) *count = 0;
        return NULL;
    }

    storage = (PathDirectoryNode**)malloc(sizeof *storage * (p.idx+1));
    if(!storage)
        Con_Error("Textures::collectDirectoryNodes: Failed on allocation of %lu bytes for new collection.", (unsigned long) (sizeof* storage * (p.idx+1)));
    return collectDirectoryNodes(namespaceId, like, count, storage);
}

static int composeAndCompareDirectoryNodePaths(const void* nodeA, const void* nodeB)
{
    ddstring_t* a = composePathForDirectoryNode(*(const PathDirectoryNode**)nodeA, TEXTURES_PATH_DELIMITER);
    ddstring_t* b = composePathForDirectoryNode(*(const PathDirectoryNode**)nodeB, TEXTURES_PATH_DELIMITER);
    int delta = stricmp(Str_Text(a), Str_Text(b));
    Str_Delete(b);
    Str_Delete(a);
    return delta;
}

/**
 * @defgroup printTextureFlags  Print Texture Flags
 * @{
 */
#define PTF_TRANSFORM_PATH_NO_NAMESPACE 0x1 /// Do not print the namespace.
/**@}*/

#define DEFAULT_PRINTTEXTUREFLAGS       0

/**
 * @param flags  @see printTextureFlags
 */
static size_t printTextures3(texturenamespaceid_t namespaceId, const char* like, int flags)
{
    const boolean printNamespace = !(flags & PTF_TRANSFORM_PATH_NO_NAMESPACE);
    uint idx, count = 0;
    PathDirectoryNode **foundTextures = collectDirectoryNodes(namespaceId, like, &count, NULL);
    PathDirectoryNode** iter;
    int numFoundDigits, numUidDigits;

    if(!foundTextures) return 0;

    if(!printNamespace)
        Con_FPrintf(CPF_YELLOW, "Known textures in namespace '%s'", Str_Text(Textures_NamespaceName(namespaceId)));
    else // Any namespace.
        Con_FPrintf(CPF_YELLOW, "Known textures");

    if(like && like[0])
        Con_FPrintf(CPF_YELLOW, " like \"%s\"", like);
    Con_FPrintf(CPF_YELLOW, ":\n");

    // Print the result index key.
    numFoundDigits = MAX_OF(3/*idx*/, M_NumDigits((int)count));
    numUidDigits = MAX_OF(3/*uid*/, M_NumDigits((int)Textures_Size()));
    Con_Printf(" %*s: %-*s %*s origin\n", numFoundDigits, "idx",
        printNamespace? 22 : 14, printNamespace? "namespace:path" : "path",
        numUidDigits, "uid");
    Con_PrintRuler();

    // Sort and print the index.
    qsort(foundTextures, (size_t)count, sizeof *foundTextures, composeAndCompareDirectoryNodePaths);

    idx = 0;
    for(iter = foundTextures; *iter; ++iter)
    {
        PathDirectoryNode* node = *iter;
        Con_Printf(" %*u: ", numFoundDigits, idx++);
        printTextureOverview(node, printNamespace);
    }

    free(foundTextures);
    return count;
}

static void printTextures2(texturenamespaceid_t namespaceId, const char* like, int flags)
{
    size_t printTotal = 0;
    // Do we care which namespace?
    if(namespaceId == TN_ANY && like && like[0])
    {
        printTotal = printTextures3(namespaceId, like, flags & ~PTF_TRANSFORM_PATH_NO_NAMESPACE);
        Con_PrintRuler();
    }
    // Only one namespace to print?
    else if(VALID_TEXTURENAMESPACEID(namespaceId))
    {
        printTotal = printTextures3(namespaceId, like, flags | PTF_TRANSFORM_PATH_NO_NAMESPACE);
        Con_PrintRuler();
    }
    else
    {
        // Collect and sort in each namespace separately.
        int i;
        for(i = TEXTURENAMESPACE_FIRST; i <= TEXTURENAMESPACE_LAST; ++i)
        {
            size_t printed = printTextures3((texturenamespaceid_t)i, like, flags| PTF_TRANSFORM_PATH_NO_NAMESPACE);
            if(printed != 0)
            {
                printTotal += printed;
                Con_PrintRuler();
            }
        }
    }
    Con_Printf("Found %lu %s.\n", (unsigned long) printTotal, printTotal == 1? "Texture" : "Textures");
}

static void printTextures(texturenamespaceid_t namespaceId, const char* like)
{
    printTextures2(namespaceId, like, DEFAULT_PRINTTEXTUREFLAGS);
}

D_CMD(ListTextures)
{
    texturenamespaceid_t namespaceId = TN_ANY;
    const char* like = NULL;
    Uri* uri = NULL;

    if(!Textures_Size())
    {
        Con_Message("There are currently no textures defined/loaded.\n");
        return true;
    }

    // "listtextures [namespace] [name]"
    if(argc > 2)
    {
        uri = Uri_New();
        Uri_SetScheme(uri, argv[1]);
        Uri_SetPath(uri, argv[2]);

        namespaceId = Textures_ParseNamespace(Str_Text(Uri_Scheme(uri)));
        if(!VALID_TEXTURENAMESPACEID(namespaceId))
        {
            Con_Printf("Invalid namespace \"%s\".\n", Str_Text(Uri_Scheme(uri)));
            Uri_Delete(uri);
            return false;
        }
        like = Str_Text(Uri_Path(uri));
    }
    // "listtextures [namespace:name]" i.e., a partial Uri
    else if(argc > 1)
    {
        uri = Uri_NewWithPath2(argv[1], RC_NULL);
        if(!Str_IsEmpty(Uri_Scheme(uri)))
        {
            namespaceId = Textures_ParseNamespace(Str_Text(Uri_Scheme(uri)));
            if(!VALID_TEXTURENAMESPACEID(namespaceId))
            {
                Con_Printf("Invalid namespace \"%s\".\n", Str_Text(Uri_Scheme(uri)));
                Uri_Delete(uri);
                return false;
            }

            if(!Str_IsEmpty(Uri_Path(uri)))
                like = Str_Text(Uri_Path(uri));
        }
        else
        {
            namespaceId = Textures_ParseNamespace(Str_Text(Uri_Path(uri)));
            if(!VALID_TEXTURENAMESPACEID(namespaceId))
            {
                namespaceId = TN_ANY;
                like = argv[1];
            }
        }
    }

    printTextures(namespaceId, like);

    if(uri) Uri_Delete(uri);
    return true;
}

D_CMD(InspectTexture)
{
    Uri* search = Uri_NewWithPath2(argv[1], RC_NULL);
    texture_t* tex;

    if(!Str_IsEmpty(Uri_Scheme(search)))
    {
        texturenamespaceid_t namespaceId = Textures_ParseNamespace(Str_Text(Uri_Scheme(search)));
        if(!VALID_TEXTURENAMESPACEID(namespaceId))
        {
            Con_Printf("Invalid namespace \"%s\".\n", Str_Text(Uri_Scheme(search)));
            Uri_Delete(search);
            return false;
        }
    }

    tex = Textures_ToTexture(Textures_ResolveUri(search));
    if(tex)
    {
        printTextureInfo(tex);
    }
    else
    {
        ddstring_t* path = Uri_ToString(search);
        Con_Printf("Unknown texture \"%s\".\n", Str_Text(path));
        Str_Delete(path);
    }
    Uri_Delete(search);
    return true;
}

#if _DEBUG
D_CMD(PrintTextureStats)
{
    texturenamespaceid_t namespaceId;

    if(!Textures_Size())
    {
        Con_Message("There are currently no textures defined/loaded.\n");
        return true;
    }

    Con_FPrintf(CPF_YELLOW, "Texture Statistics:\n");
    for(namespaceId = TEXTURENAMESPACE_FIRST; namespaceId <= TEXTURENAMESPACE_LAST; ++namespaceId)
    {
        PathDirectory* texDirectory = getDirectoryForNamespaceId(namespaceId);
        uint size;

        if(!texDirectory) continue;

        size = PathDirectory_Size(texDirectory);
        Con_Printf("Namespace: %s (%u %s)\n", Str_Text(Textures_NamespaceName(namespaceId)), size, size==1? "texture":"textures");
        PathDirectory_PrintHashDistribution(texDirectory);
        PathDirectory_Print(texDirectory, TEXTURES_PATH_DELIMITER);
    }
    return true;
}
#endif
