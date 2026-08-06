/* generated using openapi-typescript-codegen -- do not edit */
/* istanbul ignore file */
/* tslint:disable */
/* eslint-disable */
import type { PlantNutritionProfileCreate } from "../models/PlantNutritionProfileCreate";
import type { PlantNutritionProfileOut } from "../models/PlantNutritionProfileOut";
import type { PlantNutritionProfileUpdate } from "../models/PlantNutritionProfileUpdate";
import type { ResponseList_PlantNutritionProfileOut_ } from "../models/ResponseList_PlantNutritionProfileOut_";
import type { CancelablePromise } from "../core/CancelablePromise";
import { OpenAPI } from "../core/OpenAPI";
import { request as __request } from "../core/request";
export class NutritionService {
  /**
   * Get Nutrition Profiles
   * @param page
   * @param limit
   * @returns ResponseList_PlantNutritionProfileOut_ Successful Response
   * @throws ApiError
   */
  public static getNutritionProfiles(
    page: number = 1,
    limit: number = 25,
  ): CancelablePromise<ResponseList_PlantNutritionProfileOut_> {
    return __request(OpenAPI, {
      method: "GET",
      url: "/nutrition/profiles",
      query: {
        page: page,
        limit: limit,
      },
      errors: {
        422: `Validation Error`,
      },
    });
  }
  /**
   * Create Nutrition Profile
   * @param requestBody
   * @returns PlantNutritionProfileOut Successful Response
   * @throws ApiError
   */
  public static createNutritionProfile(
    requestBody: PlantNutritionProfileCreate,
  ): CancelablePromise<PlantNutritionProfileOut> {
    return __request(OpenAPI, {
      method: "POST",
      url: "/nutrition/profiles",
      body: requestBody,
      mediaType: "application/json",
      errors: {
        400: `Bad Request`,
        401: `Unauthorized`,
        403: `Forbidden`,
        422: `Validation Error`,
        500: `Internal Server Error`,
      },
    });
  }
  /**
   * Get Active Nutrition Profile
   * @returns PlantNutritionProfileOut Successful Response
   * @throws ApiError
   */
  public static getActiveNutritionProfile(): CancelablePromise<PlantNutritionProfileOut> {
    return __request(OpenAPI, {
      method: "GET",
      url: "/nutrition/profiles/active",
      errors: {
        404: `Not Found`,
        500: `Internal Server Error`,
      },
    });
  }
  /**
   * Get Nutrition Profile By Id
   * @param nutritionId
   * @returns PlantNutritionProfileOut Successful Response
   * @throws ApiError
   */
  public static getNutritionProfileById(
    nutritionId: string,
  ): CancelablePromise<PlantNutritionProfileOut> {
    return __request(OpenAPI, {
      method: "GET",
      url: "/nutrition/profiles/{nutrition_id}",
      path: {
        nutrition_id: nutritionId,
      },
      errors: {
        404: `Not Found`,
        422: `Validation Error`,
        500: `Internal Server Error`,
      },
    });
  }
  /**
   * Update Nutrition Profile
   * @param nutritionId
   * @param requestBody
   * @returns PlantNutritionProfileOut Successful Response
   * @throws ApiError
   */
  public static updateNutritionProfile(
    nutritionId: string,
    requestBody: PlantNutritionProfileUpdate,
  ): CancelablePromise<PlantNutritionProfileOut> {
    return __request(OpenAPI, {
      method: "PATCH",
      url: "/nutrition/profiles/{nutrition_id}",
      path: {
        nutrition_id: nutritionId,
      },
      body: requestBody,
      mediaType: "application/json",
      errors: {
        400: `Bad Request`,
        401: `Unauthorized`,
        403: `Forbidden`,
        404: `Not Found`,
        422: `Validation Error`,
        500: `Internal Server Error`,
      },
    });
  }
  /**
   * Delete Nutrition Profile
   * @param nutritionId
   * @returns any Successful Response
   * @throws ApiError
   */
  public static deleteNutritionProfile(
    nutritionId: string,
  ): CancelablePromise<any> {
    return __request(OpenAPI, {
      method: "DELETE",
      url: "/nutrition/profiles/{nutrition_id}",
      path: {
        nutrition_id: nutritionId,
      },
      errors: {
        401: `Unauthorized`,
        403: `Forbidden`,
        404: `Not Found`,
        422: `Validation Error`,
        500: `Internal Server Error`,
      },
    });
  }
  /**
   * Activate Nutrition Profile
   * @param nutritionId
   * @returns PlantNutritionProfileOut Successful Response
   * @throws ApiError
   */
  public static activateNutritionProfile(
    nutritionId: string,
  ): CancelablePromise<PlantNutritionProfileOut> {
    return __request(OpenAPI, {
      method: "PATCH",
      url: "/nutrition/profiles/{nutrition_id}/activate",
      path: {
        nutrition_id: nutritionId,
      },
      errors: {
        401: `Unauthorized`,
        403: `Forbidden`,
        404: `Not Found`,
        422: `Validation Error`,
        500: `Internal Server Error`,
      },
    });
  }
}
