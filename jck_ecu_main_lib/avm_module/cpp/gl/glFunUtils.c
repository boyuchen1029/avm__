#include "../../system.h"
#include "gl/glFunUtils.h"
#include "avm/GLHeader.h"
#include <stdlib.h>
#include "../../../setup/AVM_STRUCT.h"
#include "avm/fp_source.h"
#include "avm/stb_image.h"
#include <sys/stat.h>
#include <unistd.h>


/**
 * @brief Copy depth buffer from one framebuffer to another.
 * @param srcFbo Source framebuffer object.
 * @param dstFbo Destination framebuffer object.
 * @param srcW Width of the source framebuffer.
 * @param srcH Height of the source framebuffer.
 * @param dstW Width of the destination framebuffer.
 * @param dstH Height of the destination framebuffer.
 * @details This function copies the depth buffer from @p srcFbo to @p dstFbo.
 *          The source and destination buffers are specified by their
 *          respective sizes, i.e. @p srcW x @p srcH and @p dstW x @p dstH.
 *          The depth buffer is copied using GL_NEAREST filtering.
 *          After the copy, the function resets the framebuffer bindings to
 *          default.
 */
void GL_COPY_FRAMEBUFFER_DEPTH(GLuint srcFbo, GLuint dstFbo,
    int srcOffsetX, int srcOffsetY, GLsizei srcW, GLsizei srcH,
    int dstOffsetX, int dstOffsetY, GLsizei dstW, GLsizei dstH)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFbo);

    glBlitFramebuffer(
        srcOffsetX, srcOffsetY,
        srcOffsetX + (int)srcW, srcOffsetY + (int)srcH,
        dstOffsetX, dstOffsetY,
        dstOffsetX + (int)dstW, dstOffsetY + (int)dstH,
        GL_DEPTH_BUFFER_BIT,
        GL_NEAREST
    );
}

/**
 * @brief Attach a depth renderbuffer to a framebuffer.
 *
 * @param FBO Framebuffer object to which the depth renderbuffer is attached.
 * @param srcDepthObj Depth renderbuffer object to be attached.
 * @details This function attaches the depth renderbuffer specified by
 *          @p srcDepthObj to the framebuffer object specified by @p FBO.
 *          The specified depth renderbuffer is used as the depth buffer
 *          for the framebuffer. The function first binds the specified
 *          framebuffer object, then attaches the specified depth
 *          renderbuffer to it using glFramebufferRenderbuffer. Finally,
 *          the function resets the framebuffer binding to default.
 */
void GL_FBO_AttachDepthRB(GLuint FBO, GLuint srcDepthObj) 
{
    /* Bind the specified framebuffer object. */
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    /* Attach the specified depth renderbuffer to the depth attachment
     * point of the framebuffer. */
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, srcDepthObj);
    /* Reset the framebuffer binding to default. */
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


/**
 * @brief Normalizes a pixel position in a 2D image.
 *
 * @details This function normalizes the pixel position specified by
 *          @p x and @p y in a 2D image of size @p w x @p h. The
 *          normalized x and y coordinates are stored in the variables
 *          pointed to by @p dstx and @p dsty, respectively. The
 *          normalized coordinates range from 0.0 to 1.0.
 *
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @param dstx Pointer to a double where the normalized x-coordinate will be stored.
 * @param dsty Pointer to a double where the normalized y-coordinate will be stored.
 * @param w The width of the image.
 * @param h The height of the image.
 */
void GL_Normal_Pixel2glpt(double x, double y, double* dstx, double* dsty, int w, int h)
{
    /* Normalize the x-coordinate by dividing it by the image width and
     * multiplying it by 2.0, then subtracting 1.0. */
    *dstx = x  / (double)w * 2.0 - 1.0;

    /* Normalize the y-coordinate by subtracting it from the image height,
     * dividing the result by the image height, then multiplying it by
     * 2.0, and finally subtracting 1.0. */
    *dsty = 1.0 - y / (double)h * 2.0;
}

/**
 * @brief Normalize a 2D point in OpenGL coordinates to a 2D pixel position.
 * @param x The x-coordinate of the point in OpenGL coordinates.
 * @param y The y-coordinate of the point in OpenGL coordinates.
 * @param dstx Pointer to a double where the normalized x-coordinate will be stored.
 * @param dsty Pointer to a double where the normalized y-coordinate will be stored.
 * @param w The width of the image.
 * @param h The height of the image.
 * @details This function normalizes the 2D point specified by @p x and @p y in
 *          OpenGL coordinates to a 2D pixel position. The normalized
 *          x and y coordinates are stored in the variables pointed to by
 *          @p dstx and @p dsty, respectively. The normalized coordinates range
 *          from 0.0 to the image width and height, respectively.
 */
void GL_Normal_glpt2Pixel(double x, double y, double* dstx, double* dsty, int w, int h)
{
    /* Normalize the x-coordinate by adding 1.0, then multiplying by half the image width. */
    *dstx = (x + 1.0) * (double)w * 0.5;

    /* Normalize the y-coordinate by subtracting it from 1.0, then multiplying by half the image height. */
    *dsty = (-y + 1.0) * (double)h * 0.5;
}

/**
 * @brief Transform a 4-dimensional vector by a 4x4 matrix.
 * @param vec The 4-dimensional vector to be transformed.
 * @param arr16 The 4x4 matrix (represented as a 1D array of 16 floats)
 * @param result The transformed 4-dimensional vector.
 * @param isNormto2D Whether to normalize the result to 2D coordinates.
 * @details This function transforms the 4-dimensional vector specified by @p vec using the 4x4 matrix specified by @p arr16. The transformed vector is stored in the variable pointed to by @p result. If @p isNormto2D is set to true, the function normalizes the result to 2D coordinates.
 */
void GL_transform_vec4_by_arr16(VECF4 vec, float* arr16, VECF4* result, _Bool isNormto2D)
{
    result->X = vec.X * arr16[0] + vec.Y * arr16[4] + vec.Z * arr16[8]  + vec.W * arr16[12];
    result->Y = vec.X * arr16[1] + vec.Y * arr16[5] + vec.Z * arr16[9]  + vec.W * arr16[13];
    result->Z = vec.X * arr16[2] + vec.Y * arr16[6] + vec.Z * arr16[10] + vec.W * arr16[14];
    result->W = vec.X * arr16[3] + vec.Y * arr16[7] + vec.Z * arr16[11] + vec.W * arr16[15];
    float x = result->X;
    float y = result->Y;
    float z = result->Z;
    float w = result->W;
    if(isNormto2D)
    {
        float x = result->X;
        float y = result->Y;
        float z = result->Z;
        float w = result->W;
        result->X = x / w;
        result->Y = y / w;
        result->Z = z / w;
        result->W = w / w;
    }
}

