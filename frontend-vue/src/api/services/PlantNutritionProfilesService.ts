/* generated using openapi-typescript-codegen -- do not edit */
/* istanbul ignore file */
/* tslint:disable */
/* eslint-disable */
import type { CancelablePromise } from '../core/CancelablePromise';
import { OpenAPI } from '../core/OpenAPI';
import { request as __request } from '../core/request';
import type { PlantNutritionProfileCreate } from '../models/PlantNutritionProfileCreate';
import type { PlantNutritionProfileOut } from '../models/PlantNutritionProfileOut';
import type { PlantNutritionProfileUpdate } from '../models/PlantNutritionProfileUpdate';
import type { ResponseList_PlantNutritionProfileOut_ } from '../models/ResponseList_PlantNutritionProfileOut_';

export class PlantNutritionProfilesService {
    public static getNutritionProfiles(
        page: number = 1,
        limit: number = 25,
    ): CancelablePromise<ResponseList_PlantNutritionProfileOut_> {
        return __request(OpenAPI, {
            method: 'GET',
            url: '/nutrition/profiles',
            query: {
                'page': page,
                'limit': limit,
            },
        });
    }

    public static getActiveNutritionProfile(): CancelablePromise<PlantNutritionProfileOut> {
        return __request(OpenAPI, {
            method: 'GET',
            url: '/nutrition/profiles/active',
        });
    }

    public static getNutritionProfileById(
        nutritionId: string,
    ): CancelablePromise<PlantNutritionProfileOut> {
        return __request(OpenAPI, {
            method: 'GET',
            url: '/nutrition/profiles/{nutrition_id}',
            path: {
                'nutrition_id': nutritionId,
            },
        });
    }

    public static createNutritionProfile(
        requestBody: PlantNutritionProfileCreate,
    ): CancelablePromise<PlantNutritionProfileOut> {
        return __request(OpenAPI, {
            method: 'POST',
            url: '/nutrition/profiles',
            body: requestBody,
            mediaType: 'application/json',
            errors: {
                422: `Validation Error`,
            },
        });
    }

    public static updateNutritionProfile(
        nutritionId: string,
        requestBody: PlantNutritionProfileUpdate,
    ): CancelablePromise<PlantNutritionProfileOut> {
        return __request(OpenAPI, {
            method: 'PATCH',
            url: '/nutrition/profiles/{nutrition_id}',
            path: {
                'nutrition_id': nutritionId,
            },
            body: requestBody,
            mediaType: 'application/json',
            errors: {
                422: `Validation Error`,
            },
        });
    }

    public static activateNutritionProfile(
        nutritionId: string,
    ): CancelablePromise<PlantNutritionProfileOut> {
        return __request(OpenAPI, {
            method: 'PATCH',
            url: '/nutrition/profiles/{nutrition_id}/activate',
            path: {
                'nutrition_id': nutritionId,
            },
        });
    }

    public static deleteNutritionProfile(
        nutritionId: string,
    ): CancelablePromise<any> {
        return __request(OpenAPI, {
            method: 'DELETE',
            url: '/nutrition/profiles/{nutrition_id}',
            path: {
                'nutrition_id': nutritionId,
            },
        });
    }
}
