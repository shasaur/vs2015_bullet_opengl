#include "VertexBufferObject.h"

/*
Notes:
    VBO Problems.

    a) vertex must be last.
    b) collapse happens even if colour/texture etc is same.

    modes of usage

    vvvvv ok
    vcvcvcvcvcvc ok
    vctvtcvctv ok
    vctvcv fail on building vbo.

    implement defered building of the VBO's until 2nd vertex. must use .End() to finish off and build the VBO's

    Implement with 4 VBO's as vectors

    use GLM to manage vec2 and vec3

*/


static
inline
bool
FpEq( float f1, float f2 ) {
    return f1 == f2;
}

void
VertexBufferObject::Fillin(void) { // vertex and other data must exist.
    if(!hasVertices) {
        if(verbose)
            fprintf(stderr, "Generating vertex without data");
        return;
    }
    glm::vec4 v = glm::vec4(c_x, c_y, c_z, 1.);
    vertexContainer.push_back(v);
    if(hasNormals)
        normalContainer.push_back(glm::vec3(c_nx, c_ny, c_nz));
    if(hasColors)
        colorContainer.push_back(glm::vec3(c_r, c_g, c_b));
    if(hasTexCoords) {
//            printf("Pushing texture %d %f %f\n", textureContainer.size(), c_s, c_t);
        textureContainer.push_back(glm::vec2(c_s, c_t));
    }
}

//void
//VertexBufferObject::AddVertex( GLfloat x, GLfloat y, GLfloat z ) {
//    Fillin();
//}


void
VertexBufferObject::CollapseCommonVertices( bool tf ) {
    collapseCommonVertices = tf;
}

void
VertexBufferObject::SelectVAO() {
    glBindVertexArray(arrayBuffer);
}
void
VertexBufferObject::DeSelectVAO() {
    glBindVertexArray(0);
}


void
VertexBufferObject::Draw( ) {
    // much simpler
    // instanciate vao
    // draw
    // reset vao.
//    if(verbose) fprintf(stderr, "Drawing\n");
    assert(numVertices > 0);
    glBindVertexArray(arrayBuffer);
    Check("drawing 1");
    glDrawArrays(topology, 0, numVertices);
    Check("drawing 2");
    glBindVertexArray(0);
}


void
VertexBufferObject::glBegin( GLenum _topology ) {
    topology = _topology;
    Reset( );
    glBeginWasCalled = true;
}

void
VertexBufferObject::glEnd( ) {
    // now used to finalise mesh capture, create vao and build vbos.
    int attribute = 0;
    if(hasVertices) {
        Fillin();// add last data.
    } else {
        if(verbose)
            fprintf(stderr, "Drawing with no vertices.");
    }
    fprintf(stderr, "Data %d %d %d %d\n",
            vertexContainer.size(),
            colorContainer.size(),
            normalContainer.size(),
            textureContainer.size());
    glGenVertexArrays(1, &arrayBuffer);// create VAO
    glBindVertexArray(arrayBuffer);

    numVertices = vertexContainer.size();

    glGenBuffers(4, vertexBuffers);
//    if(verbose)
    fprintf(stderr, "Vertex Buffer %d created (%d %d)\n", attribute, sizeof(glm::vec4), vertexContainer.size());
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[attribute]);
    glBufferData(GL_ARRAY_BUFFER, vertexContainer.size() * sizeof(glm::vec4), vertexContainer.data(), GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)attribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
    vertexAtributesMap[attribute] = VERTEX_ATTRIBUTE;
    attribute ++;
    if(hasTexCoords) {
//        if(verbose)
        fprintf(stderr, "Texture Buffer %d created with %d enteries\n", attribute, textureContainer.size());
        assert(textureContainer.size() == vertexContainer.size());
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[attribute]);
        glBufferData(GL_ARRAY_BUFFER, textureContainer.size() * sizeof(glm::vec2), textureContainer.data(), GL_STATIC_DRAW);
        glVertexAttribPointer((GLuint)attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
        vertexAtributesMap[TEXTURE_ATTRIBUTE] = attribute;
        attribute ++;
    }
    if(hasColors) {
        if(verbose)
            fprintf(stderr, "Color Buffer %d created\n", attribute);
        assert(colorContainer.size() == vertexContainer.size());
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[attribute]);
        glBufferData(GL_ARRAY_BUFFER, colorContainer.size() * sizeof(glm::vec3), colorContainer.data(), GL_STATIC_DRAW);
        glVertexAttribPointer((GLuint)attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
        vertexAtributesMap[COLOR_ATTRIBUTE] = attribute;
        attribute ++;
    }
    if(hasNormals) {
        if(verbose)
            fprintf(stderr, "Normal Buffer %d created\n", attribute);
        assert(normalContainer.size() == vertexContainer.size());
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[attribute]);
        glBufferData(GL_ARRAY_BUFFER, normalContainer.size() * sizeof(glm::vec3), normalContainer.data(), GL_STATIC_DRAW);
        glVertexAttribPointer((GLuint)attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
        vertexAtributesMap[NORMAL_ATTRIBUTE] = attribute;
        attribute ++;
    }
    if(verbose) {
        fprintf(stderr, "%d atributes\n", attribute);
        for (std::map<int,int>::iterator it=vertexAtributesMap.begin(); it!=vertexAtributesMap.end(); ++it) {
            fprintf(stderr, "[%d] %d\n", it->first, it->second);
        }
    }

//
    glBindVertexArray(0);
    if(verbose)
        fprintf(stderr, "Vertex Array Object unbound\n");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void
VertexBufferObject::glVertex3f( GLfloat x, GLfloat y, GLfloat z ) {
    if(hasVertices) {
        Fillin();// add previous data.
    }
    hasVertices = true;
    c_x = x;
    c_y = y;
    c_z = z;
}


void
VertexBufferObject::glVertex3fv( glm::vec3 xyz ) {
    glVertex3f( xyz[0], xyz[1], xyz[2] );
}
void
VertexBufferObject::glVertex3fv( GLfloat *xyz ) {
    glVertex3f( xyz[0], xyz[1], xyz[2] );
}


void
VertexBufferObject::glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz ) {
    hasNormals = true;
    c_nx = nx;
    c_ny = ny;
    c_nz = nz;
}

void
VertexBufferObject::glNormal3fv( glm::vec3 nxyz ) {
    glNormal3f( nxyz[0], nxyz[1], nxyz[2] );
}

void
VertexBufferObject::glNormal3fv( GLfloat *nxyz ) {
    glNormal3f( nxyz[0], nxyz[1], nxyz[2] );
}

void
VertexBufferObject::glColor3f( GLfloat r, GLfloat g, GLfloat b ) {
    hasColors = true;
    c_r = r;
    c_g = g;
    c_b = b;
}


void
VertexBufferObject::glColor3fv( GLfloat *rgb ) {
    glColor3f( rgb[0], rgb[1], rgb[2] );
}
void
VertexBufferObject::glColor3fv( glm::vec3 rgb ) {
    glColor3f( rgb[0], rgb[1], rgb[2] );
}


void
VertexBufferObject::glTexCoord2f( GLfloat s, GLfloat t ) {
    hasTexCoords = true;
    c_s = s;
    c_t = t;
}


void
VertexBufferObject::glTexCoord2fv( GLfloat *st ) {
    glTexCoord2f( st[0], st[1] );
}

void
VertexBufferObject::glTexCoord2fv( glm::vec2 st ) {
    glTexCoord2f( st[0], st[1] );
}


void
VertexBufferObject::Print( FILE *fpout ) {
    //if(vertexContainer.size() < 1) {
    //    fprintf(fpout, "VBO has no vertices\n");
    //    return;
    //}
    //// always has vertices
    //fprintf(fpout, "Vertex Object %s %d vertices\n", vboName, vertexContainer.size());
    //unsigned int limit = std::min((unsigned int)100, (unsigned int) vertexContainer.size());
    //for(unsigned int i = 0; i < limit; i++) {
    //    for(int k = 0; k < 3; k++) {
    //        fprintf(fpout, " %10.2f", vertexContainer[i][k]);
    //    }
    //    fprintf(fpout, "\n");
    //}
    //if(hasColors) {
    //    fprintf(fpout, "Color Object %d vertices\n", (int) colorContainer.size());
    //    for(unsigned int i = 0; i < limit; i++) {
    //        for(int k = 0; k < 3; k++) {
    //            fprintf(fpout, " %10.2f", colorContainer[i][k]);
    //        }
    //        fprintf(fpout, "\n");
    //    }
    //}
    //if(hasTexCoords) {
    //    fprintf(fpout, "TexCoord Object %d vertices\n", (int) textureContainer.size());
    //    for(unsigned int i = 0; i < limit; i++) {
    //        for(int k = 0; k < 2; k++) {
    //            fprintf(fpout, " %10.2f", textureContainer[i][k]);
    //        }
    //        fprintf(fpout, "\n");
    //    }
    //}
}

void
VertexBufferObject::makeObj( const char *name ) {
    int v1, v2, v3, j;
    if(vertexContainer.size() < 1) {
        fprintf(stderr, "VBO has no vertices to make into obj file.\n");
        return;
    }
    int numVertices = vertexContainer.size();
    int numTriangles = numVertices/3;
    glm::vec4 p;
    FILE* objf = fopen(name, "w");
    fprintf(objf, "# Simple .obj file\n");
    for(int i = 0; i < numVertices; i++) {
        p = vertexContainer[i];
        fprintf(objf, "v %f %f %f # %d\n", p[0], p[1], p[2], i+1);
    }
    j = 1;
    for(int i= 0; i < numTriangles; i++) {
        v1 = j++;
        v2 = j++;
        v3 = j++;
        fprintf(objf, "f %5d %5d %5d # %d\n", v1, v2, v3, i+1);
    }
    fclose(objf);
}

void
VertexBufferObject::Reset( ) { // This needs work.
//    isFirstDraw = true;
    hasVertices = hasNormals = hasColors = hasTexCoords = false;
//    glPrimitiveRestartIndex( VertexBufferObject::RESTART_INDEX );
//    glEnable( GL_PRIMITIVE_RESTART );
//
//    if( parray != NULL ) {
//        delete [ ] parray;
//        parray = NULL;
//    }
//    if( earray != NULL ) {
//        delete [ ] earray;
//        earray = NULL;
//    }
//    if( pbuffer != 0 ) {
//        glDeleteBuffers( 1, &pbuffer );
//        pbuffer = 0;
//    }
//    if( ebuffer != 0 ) {
//        glDeleteBuffers( 1, &ebuffer );
//        ebuffer = 0;
//    }
//
//    PointVec.clear( );
//    PointMap.clear( );
//    ElementVec.clear( );
}


void
VertexBufferObject::RestartPrimitive( ) {
//    ElementVec.push_back( RESTART_INDEX );
}


void
VertexBufferObject::SetTol( float _tol ) {
    tol = fabs( _tol );
}


void
VertexBufferObject::SetVerbose( bool v ) {
    verbose = v;
}


void
VertexBufferObject::UseBufferObjects( bool tf ) {
    useBufferObjects = tf;

    // be sure that vbo's are supported here before we try using them:

    if( useBufferObjects   &&   ! IsExtensionSupported( "GL_ARB_vertex_buffer_object" ) ) {
        fprintf( stderr, "Vertex Buffer Objects are not supported on this system!\n" );
        useBufferObjects = false;
    }
}


// these are here to make the map functions work:
// (Do an L1 test for tolerance equality -- presume it's faster than an L2 sqrt)

bool
operator< ( const Key& k0, const Key& k1 ) {
    if( k0.x < k1.x )
        return true;

    if( k0.x > k1.x )
        return false;

    if( k0.y < k1.y )
        return true;

    if( k0.y > k1.y )
        return false;

    return  k0.z < k1.z;
};

bool operator== ( const Key& k0, const Key& k1 ) {
    return  FpEq(k0.x,k1.x)  &&  FpEq(k0.y,k1.y)  &&  FpEq(k0.z,k1.z);
};



bool
IsExtensionSupported( const char *extension ) {
    // see if the extension is bogus:

    if( extension == NULL  ||  extension[0] == '\0' )
        return false;

    GLubyte *where = (GLubyte *) strchr( extension, ' ' );
    if( where != 0 )
        return false;

    // get the full list of extensions:

    const GLubyte *extensions = glGetString( GL_EXTENSIONS );

    for( const GLubyte *start = extensions; ; ) {
        where = (GLubyte *) strstr( (const char *) start, extension );
        if( where == 0 )
            return false;

        GLubyte *terminator = where + strlen(extension);

        if( where == start  ||  *(where - 1) == ' ' )
            if( *terminator == ' '  ||  *terminator == '\0' )
                return true;
        start = terminator;
    }
    return false;
}
